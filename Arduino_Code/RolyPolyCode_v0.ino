#include <Servo.h>
#include <Wire.h>
#include <Adafruit_LSM6DSOX.h>
// #include <LSM6.h>

// Servos
//const int servo1 = 11;                    // PWM
//const int servo2 = 10;                    // PWM
//const int servo3 = 9;                     // PWM
//const int servo4 = 6;                     // PWM
Servo servos[4];
const uint8_t servoPins[4] = {6, 9, 10, 11};
const uint8_t servoOpenAngles[4] = {0, 0, 0, 0};
const uint8_t servoCloseAngles[4] = {0, 0, 0, 0};

// Locks
const uint8_t lock_v = 13;        // digital
const uint8_t lock_h = 12;        // digital

// Motors
const uint8_t wheel_left_1 = 8;   // digital
const uint8_t wheel_left_2 = 7;   // digital
const uint8_t wheel_left_speed = 5;           // PWM
const uint8_t wheel_right_1 = 4;  // digital
const uint8_t wheel_right_2 = 2;  // digital
const uint8_t wheel_right_speed = 3;          // PWM
// These are PWM pins for the encoders, 
// but we don't have enough PWM pins yet!!!
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

// Limit switch??????

bool isShellOpen = false;   // in setup the shell will open
float joystickInput;
bool xButtonPressed = false;
bool LEFT = true;
bool RIGHT = false;
bool FORWARD = true;
bool BACKWARD = false;
float speed;

void shellOpenAlignment(){
  Serial.println("Trying to align...");
  // alignment code here
}

void shellOpenClose(){
  if(isShellOpen){
    // close the shell
    shellOpenAlignment();
    digitalWrite(lock_v, LOW);
    digitalWrite(lock_h, LOW);
    for(int i = 0; i < 4; i++){
      servos[i].write(servoCloseAngles[i]);
    }
    isShellOpen = false;
  }
  else{
    // open the shell
    for(int i = 0; i < 4; i++){
      servos[i].write(servoOpenAngles[i]);
    }
    digitalWrite(lock_v, HIGH);
    digitalWrite(lock_h, HIGH);
    isShellOpen=true;
  }
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
  if (!imu_shell.begin_I2C(imu_shell_add)) {
    Serial.println("Connecting to shell IMU...");
    while (1) {
      delay(10);
    }
  }
  if (!imu_wheels.begin_I2C(imu_wheels_add)) {
    Serial.println("Connecting to wheels IMU...");
    while (1) {
      delay(10);
    }
  }
  Serial.println("IMU's are set up!");
  
  pinMode(wheel_left_1, OUTPUT);
  pinMode(wheel_left_2, OUTPUT);
  pinMode(wheel_right_1, OUTPUT);
  pinMode(wheel_right_2, OUTPUT);
  // turn motors off
  digitalWrite(wheel_left_1, LOW);
  digitalWrite(wheel_left_2, LOW);
  digitalWrite(wheel_right_1, LOW);
  digitalWrite(wheel_right_2, LOW);

  shellOpenClose();
  // // handle shell being open
  // if(isShellOpen){
  //   // activate locks
  //   digitalWrite(lock_v, HIGH);
  //   digitalWrite(lock_h, HIGH);
  //   // set servo positions
  //   for(int i = 0; i < 4; i++){
  //     servos[i].write(servoOpenAngles[i]);
  //   }
  // }
}

void runMotor(bool moveForward, bool leftMotor, int spd){
  // leftMotor: true = left, false = right
  if (leftMotor) {
    if(spd==0){
      //brake wheel
      digitalWrite(wheel_left_1, HIGH);
      digitalWrite(wheel_left_2, HIGH);
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
      //brake wheel
      digitalWrite(wheel_right_1, HIGH);
      digitalWrite(wheel_right_2, HIGH);
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
  // placeholder: let's say the controller inputs a number from -1 to 1
  float joystickInput = 0.0;
  bool joystickSide = LEFT;
  xButtonPressed=false;

  runMotor(joystickInput>0.0, joystickSide, int((joystickInput+1)*1024));   // test this line
  if(xButtonPressed){
    // stop itself, wait for a second, then open/close
    runMotor(FORWARD, LEFT, 0);
    runMotor(FORWARD, RIGHT, 0);
    delay(1000);  // this will need to change for threading!!!
    shellOpenClose();
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
  collectIMUData();
  checkModeChange();
}
