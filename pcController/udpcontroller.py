import socket
import threading
import time
import ctypes
import udpHelpers
import atexit

# ================== CONFIG ==================
PC_IP = "0.0.0.0"
PC_PORT = 5005

ARDUINO_AP_IP = "192.168.4.1"
ARDUINO_PORT = 5006

ARDUINO_IP = ARDUINO_AP_IP  # active target

MOVE_INTERVAL = 0.1  # 100 ms

# ================== SOCKET ==================
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind((PC_IP, PC_PORT))
sock.settimeout(0.1)

running = True

atexit.register(lambda: sock.close())

# ================== WINDOWS KEY INPUT ==================
user32 = ctypes.windll.user32

VK_W = 0x57
VK_A = 0x41
VK_S = 0x53
VK_D = 0x44
VK_I = 0x49
VK_U = 0x55
VK_Q = 0x51
VK_M = 0x4D 

def key_down(vk):
    return user32.GetAsyncKeyState(vk) & 0x8000


# ================== UDP ==================
def send_packet(msg, ip=None):
    target = ip if ip else ARDUINO_IP
    sock.sendto(msg.encode(), (target, ARDUINO_PORT))
    print(f"[SENT] {msg} -> {target}")


def handle_ipswap(packet):
    global ARDUINO_IP, ARDUINO_PORT

    ip = udpHelpers.get_param_value(packet, "ip")
    port = udpHelpers.get_param_value(packet, "port")

    if ip:
        ARDUINO_IP = ip
        print(f"[UPDATE] Arduino IP = {ARDUINO_IP}")

    if port:
        try:
            ARDUINO_PORT = int(port)
            print(f"[UPDATE] Arduino Port = {ARDUINO_PORT}")
        except ValueError:
            print("[WARN] Invalid port received")


def udp_listener():
    while running:
        try:
            data, addr = sock.recvfrom(1024)
            packet = data.decode()
            print(f"\n[RECV] {addr} -> {packet}")

            if packet.startswith("ipswap"):
                handle_ipswap(packet)

        except socket.timeout:
            pass
        except Exception as e:
            print("[UDP ERROR]", e)


# ================== WIFI SWITCH ==================
def switch_arduino_to_pc_wifi(arduino_ap_ssid="Arduino_Control"):
    # Get current Wi-Fi credentials
    original_ssid = udpHelpers.get_current_wifi_ssid()
    if not original_ssid:
        print("[ERROR] No Wi-Fi connected")
        return

    password = udpHelpers.get_wifi_password(original_ssid)
    if not password:
        print("[ERROR] Could not retrieve Wi-Fi password")
        return

    print(f"[INFO] Current WiFi: {original_ssid}")

    # Connect to Arduino AP
    print("[INFO] Connecting to Arduino AP...")
    udpHelpers.connect_to_wifi(arduino_ap_ssid)

    # Send credentials to Arduino
    msg = f"wifi$ssid={original_ssid}/$pass={password}/"
    send_packet(msg, ARDUINO_AP_IP)
    print("[INFO] Credentials sent to Arduino")

    time.sleep(2)

    # 4Switch back to original Wi-Fi
    print("[INFO] Switching back to original Wi-Fi...")
    udpHelpers.connect_to_wifi(original_ssid)

    print("[DONE] Arduino instructed to switch Wi-Fi")


# ================== KEY HOLD LOOP ==================
def key_hold_loop():
    global running

    print("\nControls:")
    print(" W → move forward (hold)")
    print(" A/S/D → turn/backward")
    print(" I → send ipswap scan")
    print(" U → switch Arduino WiFi")
    print(" M → send custom message")
    print(" Q → quit\n")

    last_move = 0

    while running:
        now = time.time()

        # --- Movement keys ---
        if now - last_move >= MOVE_INTERVAL:
            if key_down(VK_W):
                send_packet("move$dir=fwd/", ARDUINO_IP)
            elif key_down(VK_S):
                send_packet("move$dir=back/", ARDUINO_IP)
            elif key_down(VK_A):
                send_packet("move$dir=left/", ARDUINO_IP)
            elif key_down(VK_D):
                send_packet("move$dir=right/", ARDUINO_IP)
            last_move = now


        # --- Ipswap ---
        if key_down(VK_I):
            for i in range(0, 30):
                if i == 0:
                    target_ip = ARDUINO_AP_IP
                else:
                    target_ip = f"192.168.1.{i}"
                send_packet("ipswap", target_ip)
            time.sleep(0.5)

        # --- WiFi switch ---
        if key_down(VK_U):
            switch_arduino_to_pc_wifi()
            time.sleep(0.5)

        # --- Custom message ---
        if key_down(VK_M):
            # Pause loop briefly to allow input
            msg = input("Enter message to send: ")
            if msg.strip():
                send_packet(f"ledmsg$msg={msg}/", ARDUINO_IP)
            time.sleep(0.2)  # prevent multiple triggers on key hold

        # --- Quit ---
        if key_down(VK_Q):
            print("[EXIT]")
            running = False
            break

        time.sleep(0.02)



# ================== MAIN ==================
if __name__ == "__main__":
    print("UDP server running")

    udp_thread = threading.Thread(target=udp_listener, daemon=True)
    key_thread = threading.Thread(target=key_hold_loop, daemon=True)

    udp_thread.start()
    key_thread.start()

    try:
        while running:
            time.sleep(0.5)
    except KeyboardInterrupt:
        print("\nCtrl+C")
    finally:
        running = False
        time.sleep(0.2)
        sock.close()
        print("Socket closed")
