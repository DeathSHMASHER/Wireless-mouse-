import socket
from pynput.mouse import Controller, Button
 
# --- CONFIGURATION ---
UDP_IP = "0.0.0.0" # Listens on all available network interfaces
UDP_PORT = 4210
 
# Set up UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

mouse = Controller()

print(f"Listening for ESP32 mouse data on port {UDP_PORT}...")

# DEADZONE: Prevents cursor jitter when your hand is naturally shaking slightly
DEADZONE = 12

prev_left = 0
prev_right = 0

while True:
    try:
        # Receive packet (1024 bytes buffer is more than enough)
        data, addr = sock.recvfrom(1024) 
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

        # Handle Left Click
        if l_click == 1 and prev_left == 0:
            mouse.press(Button.left)
        elif l_click == 0 and prev_left == 1:
            mouse.release(Button.left)

        # Handle Right Click
        if r_click == 1 and prev_right == 0:
            mouse.press(Button.right)
        elif r_click == 0 and prev_right == 1:
            mouse.release(Button.right)

        prev_left = l_click
        prev_right = r_click

    except KeyboardInterrupt:
        print("\nExiting...")
        break
    except Exception as e:
        # Ignore malformed packets to keep the loop running fast
        pass