import socket
import threading
import time
import udpHelpers
from dotenv import load_dotenv
import asyncio
import websockets
import threading
import whisper
import torch
import sounddevice as sd
import numpy as np
import queue
import os
from google.genai import types
from google import genai

from google.adk.agents import LlmAgent
from google.adk.models.google_llm import Gemini
from google.adk.runners import InMemoryRunner
from google.adk.tools import AgentTool, FunctionTool, google_search
from google.adk.agents import Agent
import io
from PIL import Image

connected_phones = set()

load_dotenv()

# ================= STT CONFIG ==================

stt_queue = asyncio.Queue()   # async queue for recognized text
audio_q = queue.Queue()       # thread-safe audio buffer

def stt_worker(loop):
    SAMPLE_RATE = 16000
    SILENCE_THRESHOLD = 0.01
    SILENCE_TIME = 0.7
    MAX_UTTERANCE = 6.0

    device = "cuda" if torch.cuda.is_available() else "cpu"
    print("[STT Thread] Device:", device)

    model = whisper.load_model("small", device=device)

    def callback(indata, frames, time_info, status):
        if status:
            print(status)
        audio_q.put(indata[:, 0].copy())

    with sd.InputStream(
        samplerate=SAMPLE_RATE,
        channels=1,
        blocksize=1024,
        callback=callback,
    ):
        print("[STT Thread] 🎤 Listening...")

        speaking = False
        silence_start = None
        utterance = np.zeros(0, dtype=np.float32)

        while True:
            while not audio_q.empty():
                chunk = audio_q.get()
                rms = np.sqrt(np.mean(chunk ** 2))

                if rms > SILENCE_THRESHOLD:
                    speaking = True
                    silence_start = None
                    utterance = np.append(utterance, chunk)

                else:
                    if speaking:
                        if silence_start is None:
                            silence_start = time.time()
                        elif time.time() - silence_start > SILENCE_TIME:
                            speaking = False

                            if len(utterance) > SAMPLE_RATE * 0.5:
                                print("[STT Thread] 🧠 Transcribing...")
                                result = model.transcribe(
                                    utterance,
                                    fp16=(device == "cuda"),
                                    language="en"
                                )
                                text = result["text"].strip()
                                if text:
                                    print("[STT Thread]🎧", text)
                                    asyncio.run_coroutine_threadsafe(
                                        stt_queue.put(text), loop
                                    )

                            utterance = np.zeros(0, dtype=np.float32)
                            silence_start = None

                if speaking and len(utterance) > SAMPLE_RATE * MAX_UTTERANCE:
                    speaking = False
                    print("[STT Thread] 🧠 Transcribing (forced)...")
                    result = model.transcribe(
                        utterance,
                        fp16=(device == "cuda"),
                        language="en"
                    )
                    text = result["text"].strip()
                    if text:
                        print("[STT Thread] 🎧", text)
                        asyncio.run_coroutine_threadsafe(
                            stt_queue.put(text), loop
                        )
                    utterance = np.zeros(0, dtype=np.float32)
                    silence_start = None

            time.sleep(0.01)


# ================== UDP CONFIG ==================
PC_IP = "0.0.0.0"
PC_PORT = 5005

ARDUINO_AP_IP = "192.168.4.1"
ARDUINO_PORT = 5006

ARDUINO_IP = ""

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind((PC_IP, PC_PORT))
sock.settimeout(0.1)

running = True

def send_packet(msg, ip=None):
    target = ip if ip else ARDUINO_IP
    print(msg)
    sock.sendto(msg.encode(), (target, ARDUINO_PORT))


def handle_ipswap(packet):
    global ARDUINO_IP, ARDUINO_PORT

    ip = udpHelpers.get_param_value(packet, "ip")
    port = udpHelpers.get_param_value(packet, "port")

    if ip:
        ARDUINO_IP = ip
        print(f"[UDP Listener] Arduino IP = {ARDUINO_IP}")

    if port:
        try:
            ARDUINO_PORT = int(port)
            print(f"[UDP Listener] Arduino Port = {ARDUINO_PORT}")
        except ValueError:
            print("[UDP Listener][WARN] Invalid port received")

def udp_listener():
    while running:
        try:
            data, addr = sock.recvfrom(1024)
            packet = data.decode()
            print(f"\n[UDP Listener] {addr} -> {packet}")

            if packet.startswith("ipswap"):
                handle_ipswap(packet)

        except socket.timeout:
            pass
        except Exception as e:
            print("[UDP ERROR]", e)

def switch_arduino_to_pc_wifi(arduino_ap_ssid="Arduino_Control"):
    # Get current Wi-Fi credentials
    original_ssid = udpHelpers.get_current_wifi_ssid()
    if not original_ssid:
        print("[UDP][ERROR] No Wi-Fi connected")
        return

    password = udpHelpers.get_wifi_password(original_ssid)
    if not password:
        print("[UDP][ERROR] Could not retrieve Wi-Fi password")
        return

    print(f"[UDP][INFO] Current WiFi: {original_ssid}")

    # Connect to Arduino AP
    print("[UDP][INFO] Connecting to Arduino AP...")
    udpHelpers.connect_to_wifi(arduino_ap_ssid)

    # Send credentials to Arduino
    msg = f"wifi$ssid={original_ssid}/$pass={password}/"
    send_packet(msg, ARDUINO_AP_IP)
    print("[UDP][INFO] Credentials sent to Arduino")

    time.sleep(2)

    # 4Switch back to original Wi-Fi
    print("[UDP][INFO] Switching back to original Wi-Fi...")
    udpHelpers.connect_to_wifi(original_ssid)

    print("[UDP][DONE] Arduino instructed to switch Wi-Fi")

def trigger_ip_swap():
    for i in range(0, 30):
        if i == 0:
            target_ip = ARDUINO_AP_IP
        else:
            target_ip = f"192.168.1.{i}"
        send_packet("ipswap", target_ip)
    time.sleep(0.5)

# ================== GOOGLE ADK Agent Config ==================

os.environ["GOOGLE_API_KEY"] = os.getenv("API_KEY")
genaiClient = None

retry_config = types.HttpRetryOptions(
    attempts=5,  # Maximum retry attempts
    exp_base=7,  # Delay multiplier
    initial_delay=1,
    http_status_codes=[429, 500, 503, 504],  # Retry on these HTTP errors
)

prompt = ""

def move_rover(direction: str, duration: float):
    """
    the duration is in seconds
    direction can be: fwd, back, left, right
    fwd = forward
    back = backward
    left = rotate left
    right = rotate right
    """
    interval = 0.1
    end_time = time.time() + duration

    while time.time() < end_time:
        send_packet(f"move$dir={direction}/")
        time.sleep(interval)

    return f"Moved rover"

def message_led(msg: str):
    """
    Displays a message on Rover's LED display.
    
    :param msg: message to be displayed on the LED
    :type msg: str
    """
    send_packet(f"ledmsg$msg={msg}/")
    return "LED message sent"

def test_rover_connection():
    """
    Tests the connection to the rover by sending a ping command.
    """
    trigger_ip_swap()
    return "Ping sent to rover"

def reset_rover_connection():
    """
    Resets the rover's connection by instructing it to switch Wi-Fi networks.
    """
    switch_arduino_to_pc_wifi()
    trigger_ip_swap()
    return "Rover connection reset initiated"

# async def capture_image():
#     """
#     This function triggers capturing an image from the rover's camera.
#     """
#     for ws in list(connected_phones):
#         try:
#             await ws.send("capture")
#             return {
#                 "status": "success",
#                 "message": "I have captured the image. I am now analyzing it..."
#             }
#         except Exception as e:
#             return {"status": "error", "message": str(e)}

search_agent = Agent(
    name="search_agent",
    model=Gemini(model="gemini-2.5-flash-lite", retry_options=retry_config),
    instruction="You are a helpful assistant that can perform web searches to find information for the user.",
    tools=[google_search],
)

robot_agent = Agent(
    name="robot_agent",
    model=Gemini(model="gemini-2.5-flash-lite", retry_options=retry_config),
    instruction="""You have the ability to make conversation and control a rover robot as well. You are general assistant. You have the following abilities.

    1. move_rover helps you move the rover in different directions. The direction can be fwd, back, left, right. The duration is in seconds.
    2. If a user requests you to move in a certain direction do it for half a second by default. If they want more, they will specify.
    3. message_led helps you send a message to be displayed on the rover's LED
    4. test_rover_connection helps you test the connection to the rover. Use only when asked.
    5. reset_rover_connection helps you reset the rover's connection. Use only when asked.

    If any tool returns status "error", explain the issue to the user clearly.
    """,
    tools=[move_rover, message_led, test_rover_connection, reset_rover_connection],
)

main_agent = Agent(
    name="main_agent",
    model=Gemini(model="gemini-2.5-flash-lite", retry_options=retry_config),
    instruction="You decide which agent has to be called next based on the user request. You can call the search_agent for web searches and the robot_agent for rover control and general assistance. Always respond in a concise manner.",
    tools=[AgentTool(search_agent), AgentTool(robot_agent)],
)

def bytes_to_pil(img_bytes: bytes) -> Image.Image:
    img = Image.open(io.BytesIO(latest_image_bytes))
    img.load()  # force decode
    img = img.convert("RGB")
    return img


async def voice_agent_loop():
    global prompt
    while True:
        text = await stt_queue.get()
        print("[VOICE → AGENT]:", text)

        try:
            if(text.lower().startswith("robot")):
                query = text[len("robot"):].strip()
                prompt = query
                response = await agent_runner.run_debug(query)

                reply = "No response"

                if isinstance(response, list):
                    for event in reversed(response):
                        if (
                            event.content
                            and event.content.parts
                            and hasattr(event.content.parts[0], "text")
                        ):
                            reply = event.content.parts[0].text
                            break

                print("[AGENT REPLY]:", reply)


                for ws in list(connected_phones):
                    try:
                        await ws.send(reply)
                    except:
                        pass

        except Exception as e:
            print("[AGENT ERROR]", e)


# ==================Websocket SERVER ==================
latest_image_bytes = b""

async def ws_handler(websocket):
    print("📱 Phone connected")
    connected_phones.add(websocket)

    try:
        async for message in websocket:
            
            if isinstance(message, bytes):
                print(f"📷 Image received: {len(message)} bytes")

                # latest_image_bytes = message

                # with open("image.jpg", "wb") as f:
                #     f.write(latest_image_bytes)

                # followup_prompt = f"""
                #             Previous User request:
                #             {prompt}

                #             You requested an image. Here is the rover camera image.
                #             Analyze it and continue the task. If movement is needed, call move_rover.
                #         """
                
                # with open("image.jpg", "rb") as f:
                #     img_bytes = f.read()


                # content = Content(parts=[
                #     Part(text=followup_prompt),
                #     Part(inline_data={
                #         "mime_type": "image/jpeg",
                #         "data": img_bytes
                #     })
                # ])

                # response = await agent_runner.run(message=content)

                # reply = (
                #     response[0].content.parts[0].text
                #     if isinstance(response, list) and len(response) > 0
                #     else "No response"
                # )
                # print("[AGENT FOLLOWER]:", reply)

                # for ws in list(connected_phones):
                #     try:
                #         await ws.send(reply)
                #     except:
                #         pass               

            else:
                print("[PHONE]", message)
    except websockets.exceptions.ConnectionClosed:
        print("📱 Phone disconnected")

    finally:
        connected_phones.remove(websocket)

    

async def websocket_server_loop():
    print("[WS] Starting WebSocket server on :5008")

    async with websockets.serve(ws_handler, "0.0.0.0", 5008):
        print("[WS] WebSocket server running")

        await asyncio.Future()

# ================== MAIN ASYNC LOOP ==================

async def main_loop():
    loop = asyncio.get_running_loop()

    stt_thread = threading.Thread(
        target=stt_worker, args=(loop,), daemon=True
    )
    stt_thread.start()
    print("[STT] Thread started")
    
    await asyncio.gather(
        voice_agent_loop(),      
        websocket_server_loop()
        #any other async loops can be added here
    )

# ================== RUNNER LOOP ==================

if __name__ == "__main__":
    udp_thread = threading.Thread(target=udp_listener, daemon=True)
    udp_thread.start()

    print("[INFO] UDP listener started")

    switch_arduino_to_pc_wifi()
    time.sleep(5) 
    trigger_ip_swap()

    try:
        agent_runner = InMemoryRunner(agent=main_agent)
        print("\n[BOT] Gemini agent ready")
        asyncio.run(main_loop())
    except KeyboardInterrupt:
        print("\n[INFO] Interrupted by user")
    except Exception as e:
        print(f"\n[RUNTIME ERROR] {e}")
    finally:
        running = False 
        time.sleep(0.2)
        torch.cuda.empty_cache()
        try:
            sock.close()
            print("[CLEANUP] Socket closed safely")
        except:
            pass