#define LDR_PIN 34

void setup() {
  Serial.begin(115200);
}

void loop() {
  int ldrValue = analogRead(LDR_PIN); // Value from 0 (dark) to 4095 (bright)
  Serial.println(ldrValue);
  delay(500);
}
