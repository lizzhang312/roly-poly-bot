/*
 Example sketch for the PS5 Bluetooth library - developed by Kristian Sloth Lauszus
 For more information visit the Github repository: github.com/felis/USB_Host_Shield_2.0 or
 send me an e-mail: lauszus@gmail.com
 */

#include <PS5BT.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

USB Usb;
//USBHub Hub1(&Usb); // Some dongles have a hub inside
BTD Btd(&Usb); // You have to create the Bluetooth Dongle instance like so

/* You can create the instance of the PS5BT class in two ways */
// This will start an inquiry and then pair with the PS5 controller - you only have to do this once
// You will need to hold down the PS and Share button at the same time, the PS5 controller will then start to blink rapidly indicating that it is in pairing mode
PS5BT PS5(&Btd, PAIR);

// After that you can simply create the instance like so and then press the PS button on the device
// PS5BT PS5(&Btd);

bool printAngle = false, printTouch = false;
uint16_t lastMessageCounter = -1;
uint8_t player_led_mask = 0;
bool microphone_led = false;
uint32_t ps_timer;

//Motor setup
int in1 = 12;
int in2 = 13;
int in3 = 8;
int in4 = 9;

int speedPinA = 5;
int speedPinB = 6;

void setup() {
  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); // Halt
  }
  Serial.print(F("\r\nPS5 Bluetooth Library Started"));
//pins

pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  Serial.begin(9600);

  pinMode(speedPinA, OUTPUT);
  pinMode(speedPinB, OUTPUT);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, HIGH);
  Serial.println("working");
  // analogWrite(speedPinA, 300);
  // analogWrite(speedPinB, 300);
  digitalWrite(in1, LOW);
  digitalWrite(in4, LOW);
  // delay(10000);
}

// void runMotor(bool moveForward, bool leftMotor, int spd){
//   // leftMotor: true = left, false = right
//   if (leftMotor) {
//     if(spd==0){
//       //brake wheel
//       digitalWrite(wheel_left_1, HIGH);
//       digitalWrite(wheel_left_2, HIGH);
//     }
//     else{
//       // will need to tune the direction!
//       analogWrite(wheel_left_speed, spd);
//       digitalWrite(wheel_left_1, moveForward ? HIGH : LOW);
//       digitalWrite(wheel_left_2, moveForward ? LOW : HIGH);
//     }
//   }
//   else {
//     if(spd==0){
//       //brake wheel
//       digitalWrite(wheel_right_1, HIGH);
//       digitalWrite(wheel_right_2, HIGH);
//     }
//     else{
//       // will need to tune the direction!
//       analogWrite(wheel_right_speed, spd);
//       digitalWrite(wheel_right_1, moveForward ? LOW : HIGH);
//       digitalWrite(wheel_right_2, moveForward ? HIGH : LOW);
//     }
//   }

// }

void loop() {
  Usb.Task();

  if (PS5.connected() && lastMessageCounter != PS5.getMessageCounter()) {
    lastMessageCounter = PS5.getMessageCounter();

        // 1) Read sticks
    int leftY  = PS5.getAnalogHat(LeftHatY); // 0 to 255
    int rightY = PS5.getAnalogHat(RightHatY); // 0 to 255

    // 2) Center around zero
    int speedA = leftY  - 127;  
    int speedB = rightY - 127;
    Serial.println(speedA);
    Serial.println(speedB);
    bool dirA = (speedA > 0);
    digitalWrite(in1, dirA);
    digitalWrite(in2, !dirA);
    bool dirB = (speedB < 0);
    digitalWrite(in3, dirB);
    digitalWrite(in4, !dirB);

    // // 3) Drive Motor A
    // if (speedA >= 0) {
    //   Serial.print(F("\r\nWorking motor A"));
    //   digitalWrite(in1, HIGH);
    //   digitalWrite(in2, LOW);
    //   analogWrite(speedPinA, speedA);
    // } else {
    //   digitalWrite(in1, LOW);
    //   digitalWrite(in2, HIGH);
    //   analogWrite(speedPinA, -speedA);
    // }

    // // 4) Drive Motor B
    // if (speedB >= 0) {
    //   digitalWrite(in3, HIGH);
    //   digitalWrite(in4, LOW);
    //   analogWrite(speedPinB, speedB);
    // } else {
    //   digitalWrite(in3, LOW);
    //   digitalWrite(in4, HIGH);
    //   analogWrite(speedPinB, -speedB);
    // }

    if (PS5.getAnalogHat(LeftHatX) > 137 || PS5.getAnalogHat(LeftHatX) < 117 || PS5.getAnalogHat(LeftHatY) > 137 || PS5.getAnalogHat(LeftHatY) < 117 || PS5.getAnalogHat(RightHatX) > 137 || PS5.getAnalogHat(RightHatX) < 117 || PS5.getAnalogHat(RightHatY) > 137 || PS5.getAnalogHat(RightHatY) < 117) {
      Serial.print(F("\r\nLeftHatX: "));
      Serial.print(PS5.getAnalogHat(LeftHatX));
      Serial.print(F("\tLeftHatY: "));
      Serial.print(PS5.getAnalogHat(LeftHatY));
      Serial.print(F("\tRightHatX: "));
      Serial.print(PS5.getAnalogHat(RightHatX));
      Serial.print(F("\tRightHatY: "));
      Serial.print(PS5.getAnalogHat(RightHatY));
    }

    if (PS5.getAnalogButton(L2) || PS5.getAnalogButton(R2)) { // These are the only analog buttons on the PS5 controller
      Serial.print(F("\r\nL2: "));
      Serial.print(PS5.getAnalogButton(L2));
      Serial.print(F("\tR2: "));
      Serial.print(PS5.getAnalogButton(R2));
    }

    // Set the left trigger to resist at the right trigger's level
    static uint8_t oldR2Value = 0xFF;
    if (PS5.getAnalogButton(R2) != oldR2Value) {
      oldR2Value = PS5.getAnalogButton(R2);
    }

    // Hold the PS button for 1 second to disconnect the controller
    // This prevents the controller from disconnecting when it is reconnected,
    // as the PS button is sent when it reconnects
    if (PS5.getButtonPress(PS)) {
      if (millis() - ps_timer > 1000)
        PS5.disconnect();
    } else
      ps_timer = millis();

    if (PS5.getButtonClick(PS))
      Serial.print(F("\r\nPS"));
    if (PS5.getButtonClick(TRIANGLE)) {
      Serial.print(F("\r\nTriangle"));
    }
    if (PS5.getButtonClick(CIRCLE)) {
      Serial.print(F("\r\nCircle"));
    }
    if (PS5.getButtonClick(CROSS)) {
      Serial.print(F("\r\nCross"));

      // Set the player LEDs
      player_led_mask = (player_led_mask << 1) | 1;
      if (player_led_mask > 0x1F)
        player_led_mask = 0;
      PS5.setPlayerLed(player_led_mask); // The bottom 5 bits set player LEDs
    }
    if (PS5.getButtonClick(SQUARE)) {
      Serial.print(F("\r\nSquare"));
    }

    if (PS5.getButtonClick(UP)) {
      Serial.print(F("\r\nUp"));
    } if (PS5.getButtonClick(RIGHT)) {
      Serial.print(F("\r\nRight"));
      PS5.setLed(Blue);
    } if (PS5.getButtonClick(DOWN)) {
      Serial.print(F("\r\nDown"));
    } if (PS5.getButtonClick(LEFT)) {
      Serial.print(F("\r\nLeft"));
    }

    if (PS5.getButtonClick(L1))
      Serial.print(F("\r\nL1"));
    if (PS5.getButtonClick(L3))
      Serial.print(F("\r\nL3"));
    if (PS5.getButtonClick(R1))
      Serial.print(F("\r\nR1"));
    if (PS5.getButtonClick(R3))
      Serial.print(F("\r\nR3"));

    if (PS5.getButtonClick(CREATE))
      Serial.print(F("\r\nCreate"));

    if (printAngle) { // Print angle calculated using the accelerometer only
      Serial.print(F("\r\nPitch: "));
      Serial.print(PS5.getAngle(Pitch));
      Serial.print(F("\tRoll: "));
      Serial.print(PS5.getAngle(Roll));
    }
  }
  analogWrite(speedPinA, 50);
  analogWrite(speedPinB, 50);
  digitalWrite(in1, LOW);
  digitalWrite(in4, LOW);
}
