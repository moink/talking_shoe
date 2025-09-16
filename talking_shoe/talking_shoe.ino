#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

// CHARACTER SELECT BUTTON
#define NUMBER_OF_CHARACTERS 4   
#define BUTTON_PIN 25
#define BUTTON_DELAY 200

// LIGHT DETECTOR
#define LIGHT_DETECTOR_PIN 34
#define LIGHT_THRESHOLD 20   

// ACCELEROMETER
#define ACCELEROMETER_DELAY 100 //ms
#define MOTION_THRESHOLD 0.3

// MP3 PLAYER
#define PLAYER_VOLUME 20
#define FPSerial Serial1
#define MP3_LENGTH 3000

enum ShoeState {
  LIGHT_MOVING,
  DARK_MOVING,
  LIGHT_STATIONARY,
  DARK_STATIONARY
};

Adafruit_ADXL345_Unified ACCELEROMETER = Adafruit_ADXL345_Unified(12345);
DFRobotDFPlayerMini myDFPlayer;
volatile int character_selected = 0;
volatile bool buttonPressed = false;

void setup(){
  setup_console();
  setup_character_select_button();
  setup_accelerometer();
  setup_mp3_player();
}

void setup_console() {
  Serial.begin(115200);
}

void IRAM_ATTR handle_character_button_press() {
  buttonPressed = true;
}

void setup_character_select_button() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handle_character_button_press, FALLING);
}

void setup_accelerometer(void) {
  Wire.begin(23, 19);
  Serial.println("Accelerometer Test\n");
  if (!ACCELEROMETER.begin()) {
    Serial.println("No accelerometer detected. Check your wiring!");
    while (1);
  }
  ACCELEROMETER.setRange(ADXL345_RANGE_2_G);
}

void setup_mp3_player() {
  FPSerial.begin(9600, SERIAL_8N1, /*rx =*/16, /*tx =*/17);
  if (!myDFPlayer.begin(FPSerial, true, true)) {
    Serial.println("No mp3 player detected. Check your wiring!");
  }
  myDFPlayer.volume(PLAYER_VOLUME);
}

void loop() {
  static unsigned long timer = millis();
  static int last_character_selected = -1;
  static unsigned long lastPress = 0;
  if (buttonPressed) {
    buttonPressed = false;
    unsigned long now = millis();
    if (now - lastPress > BUTTON_DELAY) {
      character_selected = (character_selected + 1) % NUMBER_OF_CHARACTERS;
      lastPress = now;
      Serial.printf("Button count: %d\n", character_selected);
    }
  }
  if (millis() - timer > MP3_LENGTH){
    timer = millis();
    ShoeState shoe_state = get_shoe_state();
    switch(shoe_state){
      case LIGHT_MOVING:
        Serial.println("Light and moving!\n");
        myDFPlayer.play(1);
        break;
      case DARK_MOVING:
        Serial.println("Dark and moving!\n");
        myDFPlayer.play(2);
        break;
      case LIGHT_STATIONARY:
        Serial.println("Light and stationary!\n");
        myDFPlayer.play(3);
        break;
      case DARK_STATIONARY:
        Serial.println("Dark and stationary!\n");
        myDFPlayer.play(1);
        break;
    }
  }
}

ShoeState get_shoe_state() {
  bool light = is_it_light();
  bool moving = is_it_moving();
  if (light && moving) return LIGHT_MOVING;
  if (!light && moving) return DARK_MOVING;
  if (light && !moving) return LIGHT_STATIONARY;
  return DARK_STATIONARY;
}


bool is_it_light() {
  int sensorValue = analogRead(LIGHT_DETECTOR_PIN); // Value from 0 (dark) to 4095 (bright)
  return sensorValue > LIGHT_THRESHOLD;
}


bool is_it_moving() {
  sensors_event_t event; 
  ACCELEROMETER.getEvent(&event);
  float first_x = event.acceleration.x;
  float first_y = event.acceleration.y;
  float first_z = event.acceleration.z;
  delay(ACCELEROMETER_DELAY);
  ACCELEROMETER.getEvent(&event);
  float second_x = event.acceleration.x;
  float second_y = event.acceleration.y;
  float second_z = event.acceleration.z;
  float dx = second_x - first_x;
  float dy = second_y - first_y;
  float dz = second_z - first_z;
  // compute total change in acceleration
  float total_delta = sqrt(dx*dx + dy*dy + dz*dz);
  return total_delta > MOTION_THRESHOLD;
}

// select track

// select personality

// play track

// check if still playing

// wait while still playing

// check if momentary switch is pressed

// things we need
// - more jumpers

// hardware to do
// - install battery
// - install into a shoe

// recording



