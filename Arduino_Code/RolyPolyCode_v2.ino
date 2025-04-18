#include <Servo.h>
#include <Wire.h>
#include <Adafruit_LSM6DSOX.h>
#include <PS5BT.h>
#include <usbhub.h>
// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

// Servos
Servo servos[4];
const uint8_t servoPins[4] = {2, 3, 4, 5};      // PWM pins
const uint8_t servoOpenAngles[4] = {47, 146, 136, 40};  
const uint8_t servoCloseAngles[4] = {133, 61, 56, 120};

// Locks
const uint8_t lock_v = 13;        // digital
const uint8_t lock_h = 12;        // digital

// Motors
const uint8_t wheel_left_1 = 44;   // digital
const uint8_t wheel_left_2 = 45;   // digital
const uint8_t wheel_left_speed = 6;           // PWM
const uint8_t wheel_right_1 = 48;  // digital
const uint8_t wheel_right_2 = 49;  // digital
const uint8_t wheel_right_speed = 7;          // PWM
// These are PWM pins for the encoders, 
const uint8_t encoder_left_1 = 0;
const uint8_t encoder_left_2 = 0;
const uint8_t encoder_right_1 = 0;
const uint8_t encoder_right_2 = 0;

// IMU's
// LSM6 imu_shell;
// LSM6 imu_wheels;
Adafruit_LSM6DSOX imu_shell;
Adafruit_LSM6DSOX imu_wheels;
// imu addresses
const uint8_t imu_shell_add = 0x6A;   // subject to change
const uint8_t imu_wheels_add = 0x6B;   // subject to change

// Bluetooth stuff
USB Usb;
BTD Btd(&Usb);
PS5BT PS5(&Btd, PAIR);
uint16_t lastMessageCounter = -1;

bool isShellOpen = true;   
const uint8_t servo_steps = 10;
int startAngle, endAngle, intermediateAngle;
float t;

int left_spd;
int right_spd;
int LeftJoystick;
int RightJoystick;

void shellOpenAlignment(){
  Serial.println("Trying to align...");
  // alignment code here
}

void shellOpenClose(){
  if(isShellOpen){
    // close the shell
    Serial.println("closing shell...");
    //align, then lock, then close
    shellOpenAlignment();
    digitalWrite(lock_v, HIGH);
    digitalWrite(lock_h, HIGH);

    for (int step = 1; step <= servo_steps; step++){
      t = (float)step/servo_steps;
      for (int i = 0; i < 4; i++){
        startAngle = servoOpenAngles[i];
        endAngle = servoCloseAngles[i];
        intermediateAngle = startAngle + (endAngle-startAngle)*t;
        servos[i].write(intermediateAngle);
      }
      delay(50);
    }
  }
  else{
    // open the shell
    Serial.println("opening shell...");
    // open, then unlock
    for (int step = 1; step <= servo_steps; step++){
      t = (float)step/servo_steps;
      for (int i = 0; i < 4; i++){
        startAngle = servoCloseAngles[i];
        endAngle = servoOpenAngles[i];
        intermediateAngle = startAngle + (endAngle-startAngle)*t;
        servos[i].write(intermediateAngle);
      }
      delay(50);
    }
    digitalWrite(lock_v, HIGH);
    digitalWrite(lock_h, HIGH);
  }
  isShellOpen = !isShellOpen;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) {
    // comment this loop out when running the code detached!
    delay(10);
  }
  Serial.println("Serial is set up!");

  // set up servos
  for (int i = 0; i < 4; i++){
    servos[i].attach(servoPins[i]);
  }
  Serial.println("Servos are set up!");

  // set up locks
  pinMode(lock_v, OUTPUT);
  pinMode(lock_h, OUTPUT);
  Serial.println("Locks are set up!");
  
  Wire.begin();   // not sure if this is still needed... probably not
  // enable IMU's
  // if (!imu_shell.begin_I2C(imu_shell_add)) {
  //   Serial.println("Connecting to shell IMU...");
  //   while (1) {
  //     delay(10);
  //   }
  // }
  // if (!imu_wheels.begin_I2C(imu_wheels_add)) {
  //   Serial.println("Connecting to wheels IMU...");
  //   while (1) {
  //     delay(10);
  //   }
  // }
  imu_shell.begin_I2C(imu_shell_add);
  imu_wheels.begin_I2C(imu_wheels_add);
  Serial.println("IMU's are set up!");

  // Bluetooth
  #if !defined(__MIPSEL__)
    while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
  #endif
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); // Halt
  }
  Serial.print(F("\r\nPS5 Bluetooth Library Started"));
  
  pinMode(wheel_left_1, OUTPUT);
  pinMode(wheel_left_2, OUTPUT);
  pinMode(wheel_right_1, OUTPUT);
  pinMode(wheel_right_2, OUTPUT);
  // turn motors off
  digitalWrite(wheel_left_1, LOW);
  digitalWrite(wheel_left_2, LOW);
  digitalWrite(wheel_right_1, LOW);
  digitalWrite(wheel_right_2, LOW);

  // activate locks
  digitalWrite(lock_v, HIGH);
  digitalWrite(lock_h, HIGH);
  // set servo positions
  for(int i = 0; i < 4; i++){
    servos[i].write(servoOpenAngles[i]);
  }
  Serial.println("Done with setup!");
}

void runMotor(bool moveForward, bool leftMotor, int spd){
  // leftMotor: true = left, false = right
  if (leftMotor) {
    if(spd==0){
      //stop wheel
      digitalWrite(wheel_left_1, LOW);
      digitalWrite(wheel_left_2, LOW);
    }
    else{
      // will need to tune the direction!
      analogWrite(wheel_left_speed, spd);
      digitalWrite(wheel_left_1, moveForward ? HIGH : LOW);
      digitalWrite(wheel_left_2, moveForward ? LOW : HIGH);
    }
  }
  else {
    if(spd==0){
      //stop wheel
      digitalWrite(wheel_right_1, LOW);
      digitalWrite(wheel_right_2, LOW);
    }
    else{
      // will need to tune the direction!
      analogWrite(wheel_right_speed, spd);
      digitalWrite(wheel_right_1, moveForward ? LOW : HIGH);
      digitalWrite(wheel_right_2, moveForward ? HIGH : LOW);
    }
  }

}

void getControllerInfo() {
  Usb.Task();
  if (PS5.connected() && lastMessageCounter != PS5.getMessageCounter()) {
    lastMessageCounter = PS5.getMessageCounter();
    if (PS5.getButtonClick(TRIANGLE)) {
      Serial.println("Running shell open/close...");
      shellOpenClose();
    }
    LeftJoystick = PS5.getAnalogHat(LeftHatY);  // number from 0 (up) to 255 (down)
    RightJoystick = PS5.getAnalogHat(RightHatY);
    Serial.println(LeftJoystick);
    Serial.println(RightJoystick);
    if (LeftJoystick < 117){
      // left wheel forward
      Serial.println("left wheel forward!");
      left_spd = (int)(((127-LeftJoystick)*1023)/127);
      runMotor(true, true, left_spd);
    }
    else if (LeftJoystick > 137){
      // left wheel backward
      Serial.println("left wheel backward!");
      left_spd = (int)((LeftJoystick-128)*1023/127);
      runMotor(false, true, left_spd);
    }
    else {
      runMotor(true, true, 0);
    }
    if (RightJoystick < 117){
      // right wheel forward
      Serial.println("right wheel forward!");
      right_spd = (int)(((127-RightJoystick)*1023)/127);
      runMotor(true, false, right_spd);
    }
    else if (RightJoystick > 137){
      // right wheel backward
      Serial.println("right wheel backward!");
      right_spd = (int)((RightJoystick-128)*1023/127);
      runMotor(false, false, left_spd);
    }
    else{
      runMotor(true, false, 0);
    }
  }
  LeftJoystick = PS5.getAnalogHat(LeftHatY);  // number from 0 (up) to 255 (down)
  RightJoystick = PS5.getAnalogHat(RightHatY);
  if (LeftJoystick < 117){
    // left wheel forward
    Serial.println("left wheel forward!");
    left_spd = (int)((LeftJoystick*1023)/127);
    runMotor(true, true, left_spd);
  }
  else if (LeftJoystick > 137){
    // left wheel backward
    Serial.println("left wheel backward!");
    left_spd = (int)((LeftJoystick-128)*1023/127);
    runMotor(false, true, left_spd);
  }
  else if (RightJoystick < 117){
    // right wheel forward
    Serial.println("right wheel forward!");
    right_spd = (int)((RightJoystick*1023)/127);
    runMotor(true, false, right_spd);
  }
  else if (RightJoystick > 137){
    // right wheel backward
    Serial.println("right wheel backward!");
    right_spd = (int)((RightJoystick-128)*1023/127);
    runMotor(false, false, left_spd);
  }
  else{
    runMotor(true, true, 0);
    runMotor(true, false, 0);
  }
}

void collectIMUData(){
  // collect IMU data and write to SD card
  sensors_event_t s_a, s_g, w_a, w_g, temp;
  imu_shell.getEvent(&s_a, &s_g, &temp);
  imu_wheels.getEvent(&w_a, &w_g, &temp);
  // for now, just display shell values:
   Serial.print(s_a.acceleration.x);
   Serial.print(","); Serial.print(s_a.acceleration.y);
   Serial.print(","); Serial.print(s_a.acceleration.z);
   Serial.print(",");

  Serial.print(s_g.gyro.x);
  Serial.print(","); Serial.print(s_g.gyro.y);
  Serial.print(","); Serial.print(s_g.gyro.z);
  Serial.println();
  // need to code: make a lengthy array to store values, 
  // write the values to the array, when the array gets full 
  // write to the SD card 
}

bool checkSlip(){
  return false;
}

void checkModeChange(){
  // check if wheels are moving
  if(checkSlip()){
    shellOpenClose();
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  getControllerInfo();
  // collectIMUData();
  // checkModeChange();
}
