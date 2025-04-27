#include <Servo.h>
#include <Wire.h>
#include <Adafruit_LSM6DSOX.h>
#include <SPI.h>
#include <IRremote.h>
#include <SdFat.h>
#include <QuadratureEncoder.h>

// Servos
Servo servos[4];
const uint8_t servoPins[4] = {2, 3, 4, 5};      // PWM pins
const uint8_t servoOpenAngles[4] = {47, 146, 136, 40};  
const uint8_t servoCloseAngles[4] = {133, 61, 56, 120};

const int imu_led = 22;
const int sd_led = 23;
const int data_led = 24;
const int left_led = A8;
const int right_led = A9;

// Motors
const uint8_t wheel_left_1 = 46;   // digital
const uint8_t wheel_left_2 = 47;   // digital
const uint8_t wheel_left_speed = 6;           // PWM
const uint8_t wheel_right_1 = 44;  // digital
const uint8_t wheel_right_2 = 45;  // digital
const uint8_t wheel_right_speed = 7;          // PWM
// These are PWM pins for the encoders, 
const uint8_t encoder_left_1 = 18;
const uint8_t encoder_left_2 = 19;
const uint8_t encoder_right_1 = 2;
const uint8_t encoder_right_2 = 3;

Encoders leftEncoder(18, 19);
Encoders rightEncoder(2, 3);

// IMU's
Adafruit_LSM6DSOX imu_shell;
// imu addresses
const uint8_t imu_shell_add = 0x6A;   // subject to change
const uint8_t imu_wheels_add = 0x6B;   // subject to change

// SD card stuff
const uint8_t CS = 53;
const uint8_t N = 50;
int n = 0;
float imu_data[N][6];
unsigned long time_data[N];
SdFat sd;
SdFile dataFile;
char filename[] = "DATA000.TXT";
bool stopData = false;

// remote buttons
const int Fbut = 9; // up arrow
const int Bbut = 7; // down arrow
const int Sbut = 25;  // EQ
const int STOPbut = 69; // power
const int Lbut = 68;  // FBW
const int Rbut = 67;  // FFW
const int PLbut = 22; // 0
const int PRbut = 13; // ST/REPT
const int SUbut = 70; // VOL+
const int SDbut = 21; // VOL-
const int OUbut = 94; // 3
const int ODbut = 12; // 1
// but 1 = 12
// but 2 = 24
// but 3 = 94
// but 4 = 8
// but 5 = 28
// but 6 = 90
// but 7 = 66
// but 8 = 82
// but 9 = 74
// but FUNC/STOP = 71

// shelling
bool isShellOpen = false;   // start closed
const uint8_t servo_steps = 10;
int startAngle, endAngle, intermediateAngle;
float t;

int left_spd;
int right_spd;
int LeftJoystick;
int RightJoystick;
bool forward = false;
bool backward = false;

float angleX = 0.0, angleY = 0.0, angleZ = 0.0;
float speedX = 0.0, speedY = 0.0, speedZ = 0.0;

int ref_aX = 0, ref_aY = 0, ref_aZ = 0;
int ref_gX = 0, ref_gY = 0.0, ref_gZ = 0.0;

unsigned long lastIMUMillis;
unsigned long lastIRMillis;
unsigned long lastControlMillis;
unsigned long now;
sensors_event_t s_a, s_g, w_a, w_g, temp;
float dt;

long oldLeftEncoderCount;
long oldRightEncoderCount;
long error;
float Kp = 1.5;

int speed = 80;
int offset = 0;

void shellOpenClose(){
  if(isShellOpen){
    // close the shell
    Serial.println("closing shell...");

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
  }
  isShellOpen = !isShellOpen;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Serial is set up!");

  pinMode(imu_led, OUTPUT);
  pinMode(sd_led, OUTPUT);
  pinMode(data_led, OUTPUT);
  pinMode(left_led, OUTPUT);
  pinMode(right_led, OUTPUT);
  // analogWrite(left_led, speed);
  // analogWrite(right_led, speed);

  // set up servos
  for (int i = 0; i < 4; i++){
    servos[i].attach(servoPins[i]);
  }
  Serial.println("Servos are set up!");
  
  Wire.begin();   // not sure if this is still needed... probably not
  // while (!imu_shell.begin_I2C(imu_wheels_add)) {
  //   Serial.println("Connecting to shell IMU...");
  //   delay(10);
  // }
  // if (imu_shell.begin_I2C(imu_wheels_add)){
  //   // digitalWrite(imu_led, HIGH);
  //   imu_shell.getEvent(&s_a, &s_g, &temp);
  //   ref_aX = s_a.acceleration.x;
  //   ref_aY = s_a.acceleration.y;
  //   ref_aZ = s_a.acceleration.z;
  //   ref_gX = s_g.gyro.x;
  //   ref_gY = s_g.gyro.y;
  //   ref_gZ = s_g.gyro.z;
  //   Serial.println("IMU's are set up!");
  // }

  // SD card
  if (sd.begin(CS, SD_SCK_MHZ(25))){
    // digitalWrite(sd_led, HIGH);
    Serial.println("SD set up!");
  }
  for (uint16_t i = 0; i < 1000; i++){
    sprintf(filename, "DATA%03d.TXT", i);
    if (!sd.exists(filename)){
      if (dataFile.open(filename, FILE_WRITE)){
        Serial.println("data collection set up!");
        // digitalWrite(data_led, HIGH);
        break;
      }
    }
  }

  IrReceiver.begin(35);
  
  pinMode(wheel_left_1, OUTPUT);
  pinMode(wheel_left_2, OUTPUT);
  pinMode(wheel_right_1, OUTPUT);
  pinMode(wheel_right_2, OUTPUT);
  // turn motors off
  digitalWrite(wheel_left_1, LOW);
  digitalWrite(wheel_left_2, LOW);
  digitalWrite(wheel_right_1, LOW);
  digitalWrite(wheel_right_2, LOW);

  pinMode(18, INPUT_PULLUP);
  pinMode(19, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);

  // set servo positions
  for(int i = 0; i < 4; i++){
    servos[i].write(servoCloseAngles[i]);
  }

  // set references

  Serial.println("Done with setup!");
  oldLeftEncoderCount = leftEncoder.getEncoderCount();
  oldRightEncoderCount = rightEncoder.getEncoderCount();
  lastIRMillis = millis();
  lastIMUMillis = millis();
  lastControlMillis = millis();
}

void runMotor(bool moveForward, bool leftMotor, int spd){
  // leftMotor: true = left, false = right
  // spd = spd/10;
  if (leftMotor) {
    if(spd==0){
      //stop wheel
      digitalWrite(wheel_left_1, HIGH);
      digitalWrite(wheel_left_2, HIGH);
    }
    else{
      // will need to tune the direction!
      analogWrite(wheel_left_speed, spd);
      // digitalWrite(wheel_left_1, moveForward ? HIGH : LOW);
      // digitalWrite(wheel_left_2, moveForward ? LOW : HIGH);
      digitalWrite(wheel_left_1, !moveForward);
      digitalWrite(wheel_left_2, moveForward);
    }
  }
  else {
    if(spd==0){
      //stop wheel
      digitalWrite(wheel_right_1, HIGH);
      digitalWrite(wheel_right_2, HIGH);
    }
    else{
      // will need to tune the direction!
      analogWrite(wheel_right_speed, spd);
      // digitalWrite(wheel_right_1, moveForward ? LOW : HIGH);
      // digitalWrite(wheel_right_2, moveForward ? HIGH : LOW);
      digitalWrite(wheel_right_1, moveForward);
      digitalWrite(wheel_right_2, !moveForward);
    }
  }

}

void getControllerInfo2(){
  if (IrReceiver.decode()){
    IrReceiver.resume();
    int msg = IrReceiver.decodedIRData.command;
    Serial.println(msg);
    if (msg==Fbut){
        runMotor(true, true, speed + offset);
        runMotor(true, false, speed);

      forward = true;
    }
    else if (msg==Bbut){
        runMotor(false, true, speed + offset);
        runMotor(false, false, speed);
      backward = true;
    }
    else if (msg==Rbut){
      runMotor(true, true, speed);
      runMotor(true, false, 0);
    }
    else if (msg==Lbut){
      runMotor(true, false, speed);
      runMotor(true, true, 0);
    }
    else if (msg==PLbut){
      runMotor(true, true, speed);
      runMotor(false, false, speed);
    }
    else if (msg==PRbut){
      runMotor(true, false, speed);
      runMotor(false, true, speed);
    }
    else if (msg==Sbut){
      shellOpenClose();
      delay(100);
    }
    else if (msg==STOPbut){
      stopData = !stopData;
      dataFile.close();
      // digitalWrite(data_led, !stopData);
      Serial.println("data collection change");
    }
    else if (msg==SUbut){
      speed += 50;
      Serial.print("speed is ");
      Serial.println(speed);
      // analogWrite(left_led, speed+offset);
      // analogWrite(right_led, speed);
    }
    else if (msg==SDbut){
      speed -= 50;
      if(speed < 0){
        speed = 0;
      }
      Serial.print("speed is ");
      Serial.println(speed);
      // analogWrite(left_led, speed+offset);
      // analogWrite(right_led, speed);
    }
    else if (msg==OUbut){
      offset += 10;
      Serial.print("offset is ");
      Serial.println(offset);
      Serial.print("speed is ");
      Serial.println(speed);
      // analogWrite(left_led, speed+offset);
      // analogWrite(right_led, speed);
      if (offset>0){
        // analogWrite(left_led, 150);
        // analogWrite(right_led, 0);
      }
      else{
        // analogWrite(left_led, 0);
        // analogWrite(right_led, 150);
      }
    }
    else if (msg==ODbut){
      offset -= 10;
      Serial.print("offset is ");
      Serial.println(offset);
      Serial.print("speed is ");
      Serial.println(speed);
      if (offset>0){
        // analogWrite(left_led, 150);
        // analogWrite(right_led, 0);
      }
      else{
        // analogWrite(left_led, 0);
        // analogWrite(right_led, 150);
      }
      // analogWrite(left_led, speed+offset);
      // analogWrite(right_led, speed);
    }
    // else{
    //     runMotor(true, true, 0);
    //     runMotor(true, false, 0);
    // }
  }
  else{
        runMotor(true, true, 0);
        runMotor(true, false, 0);
        forward = false;
        backward = false;
  }
}

void collectIMUData(){
  // collect IMU data and write to SD card
  imu_shell.getEvent(&s_a, &s_g, &temp);
  // imu_wheels.getEvent(&w_a, &w_g, &temp);
  now = millis();
  // for now, just display shell values:
  //  Serial.print(s_a.acceleration.x);
  //  Serial.print(","); Serial.print(s_a.acceleration.y);
  //  Serial.print(","); Serial.print(s_a.acceleration.z);
  //  Serial.print(",");

  // Serial.print(s_g.gyro.x);
  // Serial.print(","); Serial.print(s_g.gyro.y);
  // Serial.print(","); Serial.print(s_g.gyro.z);
  // Serial.println();
  if (n < 100){
    imu_data[n][0] = s_a.acceleration.x - ref_aX;
    imu_data[n][1] = s_a.acceleration.y - ref_aY;
    imu_data[n][2] = s_a.acceleration.z - ref_aZ;
    imu_data[n][3] = s_g.gyro.x - ref_gX;
    imu_data[n][4] = s_g.gyro.y - ref_gY;
    imu_data[n][5] = s_g.gyro.z - ref_gZ;
    time_data[n] = now;
    n+=1;
    if (n >= 100){
      // write to SD
      // digitalWrite(data_led, HIGH);
      dataFile.open(filename, O_WRITE | O_APPEND);
      for (int i = 0; i < N; i++){
        for (int j = 0; j < 6; j++){
          dataFile.print(imu_data[i][j]);
          dataFile.print(',');
        }
        dataFile.println(time_data[i]);

      }
      dataFile.close();
      digitalWrite(data_led, LOW);
      n = 0;
    }
  }
  else{
    n = 0;
  }


  // integrate to store speed and angle
  float dt = (now-lastIMUMillis)/1000.0;
  angleX += s_g.gyro.x * dt;
  angleY += s_g.gyro.y * dt;
  angleZ += s_g.gyro.x * dt;
  speedX += s_a.acceleration.x * dt;
  speedY += s_a.acceleration.y * dt;
  speedZ += s_a.acceleration.z * dt;
}

bool checkSlip(){
  if (forward){
    if (speedX > 0.5){
      return true;
    }
  }
  if (backward){
    if (speedX < -0.5){
      return true;
    }
  }
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
  if (millis() - lastIRMillis > 200){
    getControllerInfo2();
    lastIRMillis = millis();
  }
  if (millis() - lastControlMillis > 100){
    // error = leftEncoder.getEncoderCount() + rightEncoder.getEncoderCount() - error;
    oldLeftEncoderCount = leftEncoder.getEncoderCount();
    oldRightEncoderCount = -1*rightEncoder.getEncoderCount();
    Serial.print("left:");
    Serial.println(oldLeftEncoderCount);
    Serial.print("right:");
    Serial.println(oldRightEncoderCount);
    error = oldLeftEncoderCount - oldRightEncoderCount;
    leftEncoder.setEncoderCount(0);
    rightEncoder.setEncoderCount(0);
    offset = -1*(int)(Kp*(error));
    offset = constrain(offset, -20, 20);
    if (speed+offset < 0){
      speed = -1*offset;
    }
    Serial.print("error:");
    Serial.println(error);
    // Serial.print("left:");
    // Serial.println(oldLeftEncoderCount);
    // Serial.print("right:");
    // Serial.println(oldRightEncoderCount);
    Serial.print("offset:");
    Serial.println(offset);
    lastControlMillis = millis();
  }
  // getControllerInfo2();
  // delay(200);
  // if (!stopData){
  //   collectIMUData();
  // }
  // checkModeChange();
}
