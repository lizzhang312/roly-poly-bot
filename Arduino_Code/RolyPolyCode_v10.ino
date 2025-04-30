#include <Servo.h>
#include <Wire.h>
#include <Adafruit_LSM6DSOX.h>
#include <SPI.h>
#include <IRremote.h>
#include <SdFat.h>
#include <QuadratureEncoder.h>

// Servos
Servo servos[4];
const uint8_t servoPins[4] = {12, 11, 10, 9};      // PWM pins
// const uint8_t servoOpenAngles[4] = {47, 146, 136, 40};  
// const uint8_t servoCloseAngles[4] = {133, 61, 56, 120};
const uint8_t servoOpenAngles[4] = {95, 171, 136, 40};  
const uint8_t servoCloseAngles[4] = {180, 86, 56, 120};


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
float Kp = 1.0;

int speed = 255;
int offset = 0;

IRrecv irrecv(35);
decode_results results;
String command = "";

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
  if (!stopData){
    dataFile.open(filename, FILE_WRITE);
    dataFile.print(s_a.acceleration.x);
    dataFile.print(',');
    dataFile.print(s_a.acceleration.y);
    dataFile.print(',');
    dataFile.print(s_a.acceleration.z);
    dataFile.print(',');
    dataFile.print(s_g.gyro.x);
    dataFile.print(',');
    dataFile.print(s_g.gyro.y);
    dataFile.print(',');
    dataFile.print(s_g.gyro.z);
    dataFile.print(',');
    dataFile.println(now);
    dataFile.sync();
  }
  // if (n < 100){
  //   imu_data[n][0] = s_a.acceleration.x - ref_aX;
  //   imu_data[n][1] = s_a.acceleration.y - ref_aY;
  //   imu_data[n][2] = s_a.acceleration.z - ref_aZ;
  //   imu_data[n][3] = s_g.gyro.x - ref_gX;
  //   imu_data[n][4] = s_g.gyro.y - ref_gY;
  //   imu_data[n][5] = s_g.gyro.z - ref_gZ;
  //   time_data[n] = now;
    // n+=1;
    // if (n >= N){
    //   // write to SD
    //   // digitalWrite(data_led, HIGH);
    //   dataFile.open(filename, O_WRITE | O_APPEND);
    //   for (int i = 0; i < N; i++){
    //     for (int j = 0; j < 6; j++){
    //       dataFile.print(imu_data[i][j]);
    //       dataFile.print(',');
    //     }
    //     dataFile.println(time_data[i]);

    //   }
    //   dataFile.close();
    //   digitalWrite(data_led, LOW);
    //   n = 0;
    // }
  // }
  // else{
  //   n = 0;
  // }


  // integrate to store speed and angle
  float dt = (now-lastIMUMillis)/1000.0;
  angleX += (s_g.gyro.x-ref_gX) * dt;
  angleY += (s_g.gyro.y-ref_gY) * dt;
  angleZ += (s_g.gyro.z-ref_gZ) * dt;
  speedX += (s_a.acceleration.x-ref_aX) * dt;
  speedY += (s_a.acceleration.y-ref_aY) * dt;
  speedZ += (s_a.acceleration.z-ref_aZ) * dt;
}

String translateIR() {          // takes action based on IR code received
// describing Remote IR codes 

  switch(results.value){
    case 0xFFA25D: return ("POWER"); break;
    case 0xFFE21D: return ("FUNC/STOP"); break;
    case 0xFF629D: return ("VOL+"); break;
    case 0xFF22DD: return ("FAST BACK");    break;
    case 0xFF02FD: return ("PAUSE");    break;
    case 0xFFC23D: return ("FAST FORWARD");   break;
    case 0xFFE01F: return ("DOWN");    break;
    case 0xFFA857: return ("VOL-");    break;
    case 0xFF906F: return ("UP");    break;
    case 0xFF9867: return ("EQ");    break;
    case 0xFFB04F: return ("ST/REPT");    break;
    case 0xFF6897: return ("0");    break;
    case 0xFF30CF: return ("1");    break;
    case 0xFF18E7: return ("2");    break;
    case 0xFF7A85: return ("3");    break;
    case 0xFF10EF: return ("4");    break;
    case 0xFF38C7: return ("5");    break;
    case 0xFF5AA5: return ("6");    break;
    case 0xFF42BD: return ("7");    break;
    case 0xFF4AB5: return ("8");    break;
    case 0xFF52AD: return ("9");    break;
    case 0xFFFFFFFF: return (" REPEAT");break;  

  default: 
    return "";
  }// End Case

} //END translateIR

String translateIR2() {          // takes action based on IR code received
// describing Remote IR codes 
  switch(results.value){
    case 0xFF00FF: return "VOL-";
    case 0xFF807F: return "PAUSE/PLAY";
    case 0xFF40BF: return "VOL+";
    case 0xFF20DF: return "SETUP";
    case 0xFFA05F: return "UP";
    case 0xFF609F: return "STOP/MODE";
    case 0xFF10EF: return "LEFT";
    case 0xFF906F: return "ENTER/SAVE";
    case 0xFF50AF: return "RIGHT";
    case 0xFF30CF: return "0/10+";
    case 0xFFB04F: return "DOWN";
    case 0xFF708F: return "REWIND";
    case 0xFF08F7: return "1";
    case 0xFF8877: return "2";
    case 0xFF48B7: return "3";
    case 0xFF28D7: return "4";
    case 0xFFA857: return "5";
    case 0xFF6897: return "6";
    case 0xFF18E7: return "7";
    case 0xFF9867: return "8";
    case 0xFF58A7: return "9";
    default: return "";
  }// End Case

} //END translateIR

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
  while (!imu_shell.begin_I2C(imu_wheels_add)) {
    Serial.println("Connecting to shell IMU...");
    delay(10);
  }
  if (imu_shell.begin_I2C(imu_wheels_add)){
    // digitalWrite(imu_led, HIGH);
    imu_shell.getEvent(&s_a, &s_g, &temp);
    ref_aX = s_a.acceleration.x;
    ref_aY = s_a.acceleration.y;
    ref_aZ = s_a.acceleration.z;
    ref_gX = s_g.gyro.x;
    ref_gY = s_g.gyro.y;
    ref_gZ = s_g.gyro.z;
    collectIMUData();
    Serial.println("IMU's are set up!");
  }

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

  // IrReceiver.begin(35);
  irrecv.enableIRIn();
  
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

void getControllerInfoR2(){
  // if (IrReceiver.decode()){
  if (irrecv.decode(&results)){
    //IrReceiver.resume();
    // if (translateIR() == command){
    //   return;
    // }
    command = translateIR2();
    irrecv.resume();
    // if (command==""){
    //   return;
    // }
    // if (command=="REPEAT"){
    //   command = lastCommand;
    // }
    // int msg = IrReceiver.decodedIRData.command;
    Serial.println(command);

    if (command=="UP"){
        runMotor(true, true, speed + offset);
        runMotor(true, false, speed);

      // forward = true;
      // if (forward){
      //   runMotor(true, true, 0);
      //   runMotor(true, false, 0);
      // }
      // else{
      //   runMotor(true, true, speed+offset);
      //   runMotor(true, false, speed);
      // }
      // forward = !forward;
    }
    else if (command=="DOWN"){
        runMotor(false, true, speed + offset);
        runMotor(false, false, speed);
      // backward = true;
      // if (backward){
      //   runMotor(false, true, 0);
      //   runMotor(false, false, 0);
      // }
      // else{
      //   runMotor(false, true, speed+offset);
      //   runMotor(false, false, speed);
      // }
      // backward = !backward;
    }
    else if (command=="RIGHT"){
      runMotor(true, true, speed);
      runMotor(true, false, 0);
    }
    else if (command=="LEFT"){
      runMotor(true, false, speed);
      runMotor(true, true, 0);
    }
    else if (command=="0/10+"){
      runMotor(true, true, speed);
      runMotor(false, false, speed);
    }
    else if (command=="REWIND"){
      runMotor(true, false, speed);
      runMotor(false, true, speed);
    }
    else if (command=="ENTER/SAVE"){
      shellOpenClose();
      delay(100);
    }
    else if (command=="PAUSE/PLAY"){
      stopData = !stopData;
      dataFile.close();
      // digitalWrite(data_led, !stopData);
      Serial.println("data collection change");
    }
    else if (command=="VOL+"){
      speed += 50;
      Serial.print("speed is ");
      Serial.println(speed);
      // analogWrite(left_led, speed+offset);
      // analogWrite(right_led, speed);
    }
    else if (command=="VOL-"){
      speed -= 50;
      if(speed < 0){
        speed = 0;
      }
      Serial.print("speed is ");
      Serial.println(speed);
      // analogWrite(left_led, speed+offset);
      // analogWrite(right_led, speed);
    }
    else if (command=="3"){
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
    else if (command=="1"){
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
    else if(command=="STOP/MODE"){
      runMotor(true, true, 0);
      runMotor(true, false, 0);
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

void getControllerInfo2(){
  // if (IrReceiver.decode()){
  if (irrecv.decode(&results)){
    //IrReceiver.resume();
    // if (translateIR() == command){
    //   return;
    // }
    command = translateIR();
    irrecv.resume();
    // if (command==""){
    //   return;
    // }
    // if (command=="REPEAT"){
    //   command = lastCommand;
    // }
    // int msg = IrReceiver.decodedIRData.command;
    Serial.println(command);

    if (command=="UP"){
        runMotor(true, true, speed + offset);
        runMotor(true, false, speed);

      // forward = true;
      // if (forward){
      //   runMotor(true, true, 0);
      //   runMotor(true, false, 0);
      // }
      // else{
      //   runMotor(true, true, speed+offset);
      //   runMotor(true, false, speed);
      // }
      // forward = !forward;
    }
    else if (command=="DOWN"){
        runMotor(false, true, speed + offset);
        runMotor(false, false, speed);
      // backward = true;
      // if (backward){
      //   runMotor(false, true, 0);
      //   runMotor(false, false, 0);
      // }
      // else{
      //   runMotor(false, true, speed+offset);
      //   runMotor(false, false, speed);
      // }
      // backward = !backward;
    }
    else if (command=="FAST FORWARD"){
      runMotor(true, true, speed);
      runMotor(true, false, 0);
    }
    else if (command=="FAST BACKWARD"){
      runMotor(true, false, speed);
      runMotor(true, true, 0);
    }
    else if (command=="0"){
      runMotor(true, true, speed);
      runMotor(false, false, speed);
    }
    else if (command=="ST/REPT"){
      runMotor(true, false, speed);
      runMotor(false, true, speed);
    }
    else if (command=="EQ"){
      shellOpenClose();
      delay(100);
    }
    else if (command=="POWER"){
      stopData = !stopData;
      dataFile.close();
      // digitalWrite(data_led, !stopData);
      Serial.println("data collection change");
    }
    else if (command=="VOL+"){
      speed += 50;
      Serial.print("speed is ");
      Serial.println(speed);
      // analogWrite(left_led, speed+offset);
      // analogWrite(right_led, speed);
    }
    else if (command=="VOL-"){
      speed -= 50;
      if(speed < 0){
        speed = 0;
      }
      Serial.print("speed is ");
      Serial.println(speed);
      // analogWrite(left_led, speed+offset);
      // analogWrite(right_led, speed);
    }
    else if (command=="3"){
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
    else if (command=="1"){
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
    else if(command=="FUNC/STOP"){
      runMotor(true, true, 0);
      runMotor(true, false, 0);
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
  if (millis() - lastControlMillis > 50){
    // error = leftEncoder.getEncoderCount() + rightEncoder.getEncoderCount() - error;
    oldLeftEncoderCount = leftEncoder.getEncoderCount();
    oldRightEncoderCount = -1*rightEncoder.getEncoderCount();
    // Serial.print("left:");
    // Serial.println(oldLeftEncoderCount);
    // Serial.print("right:");
    // Serial.println(oldRightEncoderCount);
    error = oldLeftEncoderCount - oldRightEncoderCount;
    leftEncoder.setEncoderCount(0);
    rightEncoder.setEncoderCount(0);
    offset = -1*(int)(Kp*(error));
    offset = constrain(offset, -50, 50);
    if (speed+offset < 0){
      speed = -1*offset;
    }
    // Serial.print("error:");
    // Serial.println(error);
    // Serial.print("left:");
    // Serial.println(oldLeftEncoderCount);
    // Serial.print("right:");
    // Serial.println(oldRightEncoderCount);
    // Serial.print("offset:");
    // Serial.println(offset);
    lastControlMillis = millis();
  }
  // getControllerInfo2();
  // delay(200);
  collectIMUData();
  // if (!stopData){
  //   collectIMUData();
  // }
  // checkModeChange();
}
