#include <Servo.h>

Servo myServo;  // Create a Servo object
int analogInput = 0; // Variable to store the user-defined analog input

void setup() {
    Serial.begin(9600); // Start serial communication
    myServo.attach(11); // Attach the servo to pin 9
    // Serial.println("Enter an analog value (0-1023):");

}

void loop() {
  if(Serial.available() > 0){
    char c = Serial.peek();
    if(isDigit(c) || c=='-'){
      int ang = Serial.parseInt();
      Serial.println(ang);
      myServo.write(ang);
      delay(100);
    }
    else{
      Serial.read();
    }
  }
//    for(int i=0;i<180;i+=30){
//      myServo.write(i);
//      Serial.println(i);
//      delay(1000);
//    }
//    if (Serial.available()) { // Check if data is available in Serial Monitor
//        analogInput = Serial.parseInt(); // Read integer from Serial Monitor
//        if (analogInput >= 0 && analogInput <= 1023) {
//            int servoAngle = map(analogInput, 0, 1023, 0, 180); // Map input to servo angle
//            myServo.write(servoAngle); // Set servo position
//            Serial.print("Analog Input: ");
//            Serial.print(analogInput);
//            Serial.print(" -> Servo Angle: ");
//            Serial.println(servoAngle);
//            delay(200);
//        } else {
//            Serial.println("Invalid input! Please enter a value between 0 and 1023.");
//        }
//    }
}



////int in1 = 13;
////int in2 = 12;
////int ENA = 9;
//int servo_pin = 9;
//
//void setup() {
//  // put your setup code here, to run once:
////  pinMode(in1, OUTPUT);
////  pinMode(in2, OUTPUT);
////  pinMode(ENA, OUTPUT);
////  digitalWrite(in1, HIGH);
////  digitalWrite(in2, HIGH);
////  Serial.begin(9600);
////  Serial.println("start");
//  pinMode(servo_pin, OUTPUT);
//}
//
//void loop() {
//  // put your main code here, to run repeatedly:
////  digitalWrite(in1, HIGH);
////  digitalWrite(in2, LOW);
////  Serial.println("150");
////  analogWrite(ENA, 150);
////  delay(2000);
////  Serial.println("50");
////  analogWrite(ENA, 50);
////  delay(2000);
////  Serial.println("0");
////  analogWrite(ENA, 0);
////  delay(2000);
//}
