#define BUTTON_PIN 25
#define NUMBER_OF_CHARACTERS 4   
#define BUTTON_DELAY 200

volatile int buttonCounter = 0;

void IRAM_ATTR handleButton() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > BUTTON_DELAY) {
    buttonCounter = (buttonCounter + 1) % NUMBER_OF_CHARACTERS;
    lastInterruptTime = interruptTime;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Trigger on falling edge (button press)
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButton, FALLING);
}

void loop() {
  // Just read the current count
  static int lastValue = -1;

  if (buttonCounter != lastValue) {
    lastValue = buttonCounter;
    Serial.printf("Button count: %d\n", buttonCounter);
  }

}