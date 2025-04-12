#include <Servo.h>
#include <Wire.h>
#include <LSM6.h>

// Servos
//const int servo1 = 11;                    // PWM
//const int servo2 = 10;                    // PWM
//const int servo3 = 9;                     // PWM
//const int servo4 = 6;                     // PWM
Servo servos[4];
const int servoPins[4] = {6, 9, 10, 11};
const int servoOpenAngles[4] = {0, 0, 0, 0};
const int servoCloseAngles[4] = {0, 0, 0, 0};

// Locks
const int lock_v = 13;        // digital
const int lock_h = 12;        // digital

// Motors
const int wheel_left_1 = 8;   // digital
const int wheel_left_2 = 7;   // digital
const int wheel_left_speed = 5;           // PWM
const int wheel_right_1 = 4;  // digital
const int wheel_right_2 = 2;  // digital
const int wheel_right_speed = 3;          // PWM
// These are PWM pins for the encoders, 
// but we don't have enough PWM pins yet!!!
const int encoder_left_1 = -1;
const int encoder_left_2 = -1;
const int encoder_right_1 = -1;
const int encoder_right_2 = -1;

// IMU's
LSM6 imu_shell;
LSM6 imu_wheels;
// imu addresses

// Bluetooth stuff

// Limit switch??????

bool isShellOpen = true;
float joystickInput;
bool xButtonPressed = false;
bool isSlipping;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // set up servos
  for (int i = 0; i < 4; i++){
    servos[i].attach(servoPins[i]);
  }

  // set up locks
  pinMode(lock_v, OUTPUT);
  pinMode(lock_h, OUTPUT);
  
  Wire.begin();
  // figure out how to initialize IMU's based on addresses
  // enable IMU's
  
  // turn motors off
  pinMode(wheel_left_1, OUTPUT);
  pinMode(wheel_left_2, OUTPUT);
  pinMode(wheel_right_1, OUTPUT);
  pinMode(wheel_right_2, OUTPUT);
  digitalWrite(wheel_left_1, LOW);
  digitalWrite(wheel_left_2, LOW);
  digitalWrite(wheel_right_1, LOW);
  digitalWrite(wheel_right_2, LOW);


  // handle shell being open
  if(isShellOpen){
    // activate locks
    digitalWrite(lock_v, HIGH);
    digitalWrite(lock_h, HIGH);
    // set servo positions
    for(int i = 0; i < 4; i++){
      servos[i].write(servoOpenAngles[i]);
    }
  }
}

void shellOpenClose(){
  if(isShellOpen){
    // need alignment code
    digitalWrite(lock_v, LOW);
    digitalWrite(lock_h, LOW);
    for(int i = 0; i < 4; i++){
      servos[i].write(servoCloseAngles[i]);
    }
  }
  else{
    for(int i = 0; i < 4; i++){
      servos[i].write(servoOpenAngles[i]);
    }
    digitalWrite(lock_v, HIGH);
    digitalWrite(lock_h, HIGH);
  }
}

void runMotor(bool dir, bool motor, int spd){
  if(spd==0){
    //brake wheels
    digitalWrite(wheel_left_1, HIGH);
    digitalWrite(wheel_left_2, HIGH);
  }
  else{
    analogWrite(wheel_left_speed, spd);
    digitalWrite(wheel_left_1, HIGH);
    digitalWrite(wheel_left_2, LOW);
  }
}

void getControllerInfo() {
  // placeholder: let's say the controller inputs a number from -1 to 1
  joystickInput = 0.0;
  xButtonPressed=false;
  if(joystickInput<0.0){
    runMotor(true, true, joystickInput*100);
  }
  else{
    runMotor(false, true, joystickInput*100);
  }
  if(xButtonPressed){
    // stop itself, wait for a second, then open/close
    runMotor(true, true, 0);
    delay(1000);  // this will need to change for threading!!!
    shellOpenClose();
  }
}

void collectIMUData(){
  // collect IMU data and write to SD card
}

void checkModeChange(){
  // check if wheels are moving
  if(isSlipping){
    shellOpenClose();
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  getControllerInfo();
  collectIMUData();
  checkModeChange();
}
