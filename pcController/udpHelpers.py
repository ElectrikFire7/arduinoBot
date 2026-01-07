import subprocess
import re
import time

def get_param_value(packet: str, param_name: str) -> str:
    key = f"${param_name}="
    key_index = packet.find(key)

    if key_index == -1:
        return ""

    value_start = key_index + len(key)
    value_end = packet.find("/", value_start)

    if value_end == -1:
        return ""

    return packet[value_start:value_end]

def get_current_wifi_ssid():
    result = subprocess.check_output(
        ["netsh", "wlan", "show", "interfaces"],
        encoding="utf-8"
    )
    match = re.search(r"SSID\s*:\s(.+)", result)
    return match.group(1).strip() if match else None

def get_wifi_password(ssid):
    result = subprocess.check_output(
        ["netsh", "wlan", "show", "profile", ssid, "key=clear"],
        encoding="utf-8",
        errors="ignore"
    )
    match = re.search(r"Key Content\s*:\s(.+)", result)
    return match.group(1).strip() if match else None

def connect_to_wifi(ssid):
    subprocess.call(["netsh", "wlan", "connect", f"name={ssid}"])
    time.sleep(5)  # allow connection time
