// === Encoder pin definitions ===
// Update these if you rewired for different pins:
const uint8_t ENC_L_A = 18;  // Left encoder Channel A
const uint8_t ENC_L_B = 19;  // Left encoder Channel B
const uint8_t ENC_R_A = 2;   // Right encoder Channel A
const uint8_t ENC_R_B = 3;   // Right encoder Channel B

// === Encoder counters ===
volatile long leftCount  = 0;
volatile long rightCount = 0;

void handleLeft() {
  bool A = digitalRead(ENC_L_A);
  bool B = digitalRead(ENC_L_B);
  leftCount += (A == B) ? +1 : -1;
}

void handleRight() {
  bool A = digitalRead(ENC_R_A);
  bool B = digitalRead(ENC_R_B);
  rightCount += (A == B) ? +1 : -1;
}

void setup() {
  Serial.begin(9600);
  while (!Serial) { delay(10); }

  // Configure encoder pins
  pinMode(ENC_L_A, INPUT_PULLUP);
  pinMode(ENC_L_B, INPUT_PULLUP);
  pinMode(ENC_R_A, INPUT_PULLUP);
  pinMode(ENC_R_B, INPUT_PULLUP);

  // Attach both A and B for each encoder to catch all edges
  attachInterrupt(digitalPinToInterrupt(ENC_L_A), handleLeft,  CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_L_B), handleLeft,  CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_R_A), handleRight, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_R_B), handleRight, CHANGE);

  Serial.println("Encoder test started. Hand-turn the shafts!");
}

void loop() {
  static unsigned long lastPrint = 0;
  unsigned long now = millis();

  // Print every 500 ms
  if (now - lastPrint >= 500) {
    noInterrupts();
      long L = leftCount;
      long R = rightCount;
      // Optionally reset to zero each print:
      leftCount  = 0;
      rightCount = 0;
    interrupts();

    Serial.print("Left pulses:  ");
    Serial.print(L);
    Serial.print("    |    Right pulses:  ");
    Serial.println(R);

    lastPrint = now;
  }
}
