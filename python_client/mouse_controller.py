import socket
import base64
from pynput.mouse import Controller, Button

# --- AUTHOR & METADATA ---
__author__ = "Shahriyar Taufik"
__github__ = "https://github.com/DeathSHMASHER"

# Hidden encoded developer signature
_DEV_SIG = (
    b"PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT0K"
    b"ICAgICAgIEVTUDMyIFdpcmVsZXNzIEFpciBNb3VzZSBDb250cm9sbGVyCiAgICAgICAgIERl"
    b"dmVsb3BlZCBieTogU2hhaHJpeWFyIFRhdWZpawogICAgR2l0SHViOiBodHRwczovL2dpdGh1"
    b"Yi5jb20vRGVhdGhTSE1BU0hFUgo9PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09PT09"
    b"PT09PT09PT09PT09PT09PT09PQ=="
)

def _print_banner():
    try:
        print("\n" + base64.b64decode(_DEV_SIG).decode("utf-8") + "\n", flush=True)
    except Exception:
        print(f"\n[ ESP32 Air Mouse | Developed by {__author__} | GitHub: {__github__} ]\n", flush=True)

# Display developer signature in terminal
_print_banner()

# Check for author / help flag
import sys
if len(sys.argv) > 1 and sys.argv[1].lower() in ["--author", "-a", "--about", "--version", "-v"]:
    print(f"Project: {__file__}")
    print(f"Lead Developer: {__author__}")
    print(f"GitHub Profile: {__github__}")
    sys.exit(0)

# --- CONFIGURATION ---
UDP_IP = "0.0.0.0" # Listens on all available network interfaces
UDP_PORT = 4210

# Set up UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

mouse = Controller()

print(f"Listening for ESP32 mouse data on port {UDP_PORT}...\n", flush=True)

# DEADZONE: Prevents cursor jitter when your hand is naturally shaking slightly
DEADZONE = 12

prev_left = 0
prev_right = 0
first_packet = True

while True:
    try:
        # Receive packet (1024 bytes buffer is more than enough)
        data, addr = sock.recvfrom(1024) 
        if first_packet:
            print(f"Connected to ESP32 at {addr[0]}! Receiving live mouse data.")
            first_packet = False 
        msg = data.decode('utf-8').strip()
        
        # Parse the "X,Y,L,R" string
        try:
            dx_str, dy_str, l_str, r_str = msg.split(',')
        except ValueError:
            # Fallback for old ESP32 code sending just "X,Y"
            dx_str, dy_str = msg.split(',')
            l_str, r_str = "0", "0"
            
        dx = int(dx_str)
        dy = int(dy_str)
        l_click = int(l_str)
        r_click = int(r_str)

        # Apply deadzone filtering
        if abs(dx) <= DEADZONE:
            dx = 0
        if abs(dy) <= DEADZONE:
            dy = 0

        # Move the mouse relatively from its current position
        if dx != 0 or dy != 0:
            # Note: Depending on how you mount the MPU6050 on your hand, 
            # you might need to invert these axes (e.g., -dx or -dy)
            mouse.move(-dx, dy) 

        # Handle Left Click (Touch Sensor 1) - supports single tap, double tap, and hold-to-drag
        if l_click == 1 and prev_left == 0:
            mouse.press(Button.left)
        elif l_click == 0 and prev_left == 1:
            mouse.release(Button.left)

        # Handle Right Click (Touch Sensor 2)
        if r_click == 1 and prev_right == 0:
            mouse.press(Button.right)
        elif r_click == 0 and prev_right == 1:
            mouse.release(Button.right)

        prev_left = l_click
        prev_right = r_click

    except KeyboardInterrupt:
        print(f"\nExiting... Project created by {__author__} ({__github__})")
        break
    except Exception as e:
        # Ignore malformed packets to keep the loop running fast
        pass