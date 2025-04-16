int in1 = 12;
int in2 = 13;
int in3 = 8;
int in4 = 9;

int speedPinA = 11;
int speedPinB = 10;

void setup() {
  // put your setup code here, to run once:
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
  delay(10000);
}

void loop() {
  // put your main code here, to run repeatedly:
  analogWrite(speedPinA, 50);
  analogWrite(speedPinB, 50);
  digitalWrite(in1, LOW);
  digitalWrite(in4, LOW);
}
