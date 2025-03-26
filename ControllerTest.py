import sys
import pygame

# Initialize pygame and its joystick module
pygame.init()
pygame.joystick.init()

# Check if a joystick is connected
if pygame.joystick.get_count() == 0:
    print("No joystick connected.")
    exit()

# Use the first joystick (index 0)
joystick = pygame.joystick.Joystick(0)
joystick.init()
print(f"Connected joystick: {joystick.get_name()}")

# Mapping of button indices to PS5 (DualSense) button names.
# Adjust these values if your controller mapping differs.
button_names = {
    0: "X",
    1: "Circle",
    2: "Square",
    3: "Triangle",
    4: "L1",
    5: "R1",
    6: "L2",
    7: "R2",
    8: "Share",
    9: "Options",
    10: "L3",
    11: "R3",
    12: "PS",
    13: "Touchpad"
}

# NEW: Define deadzone threshold
deadzone = 0.10

# Initialize left and right joystick Y positions only
left_stick_y = 0.0   # left stick: axis 1 (Y)
right_stick_y = 0.0  # right stick: axis 3 (Y)

print("Press buttons on your controller (Ctrl+C to exit).")

try:
    # Main event loop
    while True:
        for event in pygame.event.get():
            # Button pressed
            if event.type == pygame.JOYBUTTONDOWN:
                btn = event.button
                btn_name = button_names.get(btn, f"Button {btn}")
                print(f"{btn_name} pressed")
            elif event.type == pygame.JOYAXISMOTION:
                axis = event.axis
                value = event.value
                # Apply deadzone: if within ±deadzone, treat value as 0.
                if abs(value) < deadzone:
                    value = 0.12
                # Process only Y-axis events and flip sign
                if axis == 1:  # Left stick Y axis
                    left_stick_y = -value
                    print(f"Left joystick Y: {left_stick_y:.2f}")
                elif axis == 3:  # Right stick Y axis
                    right_stick_y = -value
                    print(f"Right joystick Y: {right_stick_y:.2f}")

            # Button released (if needed)
            # elif event.type == pygame.JOYBUTTONUP:
            #     btn = event.button
            #     btn_name = button_names.get(btn, f"Button {btn}")
            #     print(f"{btn_name} released")
        # Brief pause to reduce CPU usage
        pygame.time.wait(10)
except KeyboardInterrupt:
    print("Exiting...")
