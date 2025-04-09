// write code that will run the motor when the IMU says the thing is turned




#include <Servo.h>
#include <Wire.h>
#include <LSM6.h>

Servo myServo;
LSM6 imu;
//const int servoPin = 11;
//const int magnetPin = 10;

const int IN1 = 13;
const int IN2 = 12;

char imu_data[80];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
  // myServo.write(0);
  if (!imu.init())
  {
    Serial.println("Failed to detect and initialize IMU!");
    while (1);
  }
  imu.enableDefault();
//  pinMode(magnetPin, OUTPUT);
//  digitalWrite(magnetPin, LOW);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  Serial.print("start");
}

int threshold = 1000;

void loop() {
  // put your main code here, to run repeatedly:
  imu.read();
  snprintf(imu_data, sizeof(imu_data), "A: %6d %6d %6d    G: %6d %6d %6d",
    imu.a.x, imu.a.y, imu.a.z,
    imu.g.x, imu.g.y, imu.g.z);
  Serial.println(imu_data);
  int xval = imu.a.z;
  if(xval<threshold && xval>-1*threshold){
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      Serial.println("go!");
  }
  else{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    Serial.println("STOP!");
  }
  // digitalWrite(magnetPin, HIGH);
  delay(100);
}
