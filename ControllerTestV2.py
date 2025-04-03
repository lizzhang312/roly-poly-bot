import sys
import pygame
import RPi.GPIO as GPIO
import time

# Initialize pygame and joystick
pygame.init()
pygame.joystick.init()

# Check if joystick is connected
if pygame.joystick.get_count() == 0:
    print("No joystick connected.")
    exit()

joystick = pygame.joystick.Joystick(0)
joystick.init()
print(f"Connected joystick: {joystick.get_name()}")

# GPIO setup
IN1 = 17  # GPIO pin for motor direction 1
IN2 = 18  # GPIO pin for motor direction 2
ENA = 12  # GPIO pin for motor speed (PWM)

GPIO.setmode(GPIO.BCM)
GPIO.setup(IN1, GPIO.OUT)
GPIO.setup(IN2, GPIO.OUT)
GPIO.setup(ENA, GPIO.OUT)

# Setup PWM on ENA
pwm = GPIO.PWM(ENA, 1000)  # 1kHz frequency
pwm.start(0)  # Start with 0% duty cycle

# Deadzone threshold
deadzone = 0.10
max_duty_cycle = 50  # Limit max speed to 50% of full power

print("Use left stick Y to control motor (Ctrl+C to exit).")

try:
    while True:
        for event in pygame.event.get():
            if event.type == pygame.JOYAXISMOTION:
                axis = event.axis
                value = event.value

                # Apply deadzone
                if abs(value) < deadzone:
                    value = 0

                if axis == 1:  # Left stick Y-axis
                    motor_speed = abs(value) * max_duty_cycle  # Scale to 50% max
                    
                    if value > 0:
                        # Clockwise rotation
                        GPIO.output(IN1, GPIO.HIGH)
                        GPIO.output(IN2, GPIO.LOW)
                    elif value < 0:
                        # Counter-clockwise rotation
                        GPIO.output(IN1, GPIO.LOW)
                        GPIO.output(IN2, GPIO.HIGH)
                    else:
                        # Stop motor
                        GPIO.output(IN1, GPIO.LOW)
                        GPIO.output(IN2, GPIO.LOW)

                    # Update motor speed
                    pwm.ChangeDutyCycle(motor_speed)
                    print(f"Motor speed: {motor_speed:.2f}% {'CW' if value > 0 else 'CCW' if value < 0 else 'STOP'}")

        pygame.time.wait(10)
except KeyboardInterrupt:
    print("Exiting...")
    pwm.stop()
    GPIO.cleanup()
