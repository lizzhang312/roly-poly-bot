import subprocess

def enable_bluetooth_pairing():
    try:
        subprocess.run(["bluetoothctl", "discoverable", "on"], check=True)
        subprocess.run(["bluetoothctl", "pairable", "on"], check=True)
        print("Bluetooth pairing enabled.")
    except subprocess.CalledProcessError as e:
        print(f"Error enabling Bluetooth pairing: {e}")

if __name__ == "__main__":
    enable_bluetooth_pairing()