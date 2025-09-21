#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

// CHARACTER SELECT BUTTON
constexpr uint8_t NUMBER_OF_CHARACTERS = 3;
constexpr uint8_t BUTTON_PIN = 25;
constexpr uint16_t BUTTON_DELAY = 200; // ms

// LIGHT DETECTOR
constexpr uint8_t LIGHT_DETECTOR_PIN = 34;
constexpr uint16_t LIGHT_THRESHOLD = 20;

// ACCELEROMETER
constexpr uint16_t ACCELEROMETER_DELAY = 100; // ms
constexpr float MOTION_THRESHOLD = 0.3f;

// MP3 PLAYER
constexpr uint8_t PLAYER_VOLUME = 25; // 1 to 30
#define FPSerial Serial1   

// ARRAY SIZING
constexpr uint8_t MAX_TRACKS_PER_STATE = 6;
constexpr uint16_t MAX_TRACK_LENGTH_MS = 13000;

enum class SensorState : uint8_t {
    LIGHT_STATIONARY,
    LIGHT_MOVING,
    DARK_STATIONARY,
    DARK_MOVING
};

enum Activity {
    DOING_NOTHING = 0, 
    PUTTING_ON,
    WORN_STANDING_STILL,
    WALKING, 
    JUST_REMOVED, 
    NUM_ACTIVITIES   // always the last entry, sentinel value
};

Adafruit_ADXL345_Unified ACCELEROMETER = Adafruit_ADXL345_Unified(12345);
DFRobotDFPlayerMini myDFPlayer;
volatile int character_selected = 0;
volatile bool buttonPressed = false;

struct Track {
    uint16_t track_number;    // DFPlayer track number
    unsigned long length_ms;  // length of track in milliseconds
};

struct StateTracks {
    Track tracks[MAX_TRACKS_PER_STATE];
    uint8_t number_of_tracks; // how many valid tracks are defined
};

struct ActivityResult {
    Activity activity;
    bool light_transition_handled;
};

StateTracks track_table[NUMBER_OF_CHARACTERS][NUM_ACTIVITIES] = {
    // Character 0: Matt
    {
        // DOING_NOTHING (0)
        { { {1, 8277}, {2, 12331}, {4, 7680} }, 3 },
        // PUTTING_ON (1)
        { { {3, 4096}, {5, 3925}, {6, 3328} }, 3 },
        // WORN_STANDING_STILL (2)
        { { {8, 4096}, {9, 3925}, {11, 3883}, {12, 4139} }, 4 },
        // WALKING (3)
        { { {7, 2219}, {13, 3797}, {14, 3627}, {15, 4523} }, 4 },
        // JUST_REMOVED (4)
        { { {10, 5675}, {16, 8320}, {17, 3925} }, 3 }
    },

    // Character 1: Tina
    {
        // DOING_NOTHING (0)
        { { {18, 6415}, {19, 9101}, {20, 6366}, {21, 7211}, {22, 11936} }, 5 },
        // PUTTING_ON (1)
        { { {23, 3357}, {24, 7735}, {25, 12697}, {26, 7151}, {27, 5254} }, 5 },
        // WORN_STANDING_STILL (2)
        { { {28, 8397}, {29, 7007}, {30, 6006}, {31, 10177}, {32, 3504} }, 5 },
        // WALKING (3)
        { { {33, 3034}, {34, 8540}, {35, 7697}, {36, 6742}, {37, 7585}, {38, 4045} }, 6 },
        // JUST_REMOVED (4)
        { { {39, 4982}, {40, 2931}, {41, 2931}, {42, 3334} }, 4 }
    },

    // Character 2: Harry
    {
        // DOING_NOTHING (0)
        { { {43, 10841}, {44, 3762}, {45, 8751} }, 3 },
        // PUTTING_ON (1)
        { { {46, 4493}, {47, 2064}, {48, 5068} }, 3 },
        // WORN_STANDING_STILL (2)
        { { {49, 5590}, {50, 6139}, {51, 8333} }, 3 },
        // WALKING (3)
        { { {52, 6060}, {53, 3997}, {54, 10606} }, 3 },
        // JUST_REMOVED (4)
        { { {55, 5068}, {56, 4441}, {57, 13793} }, 3 }
    }
};

void setup(){
  setup_console();
  setup_character_select_button();
  setup_accelerometer();
  setup_mp3_player();
  randomSeed(9);
  // randomSeed(analogRead(A0) + millis());
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
  delay(1000);
  FPSerial.begin(9600, SERIAL_8N1, /*rx =*/16, /*tx =*/17);
  if (!myDFPlayer.begin(FPSerial, true, true)) {
    Serial.println("No mp3 player detected. Check your wiring!");
  }
  myDFPlayer.volume(PLAYER_VOLUME);
}

void loop() {
    static unsigned long playback_start_time = 0;
    static unsigned long current_track_length = 0;
    static unsigned long last_recorded_button_press = 0;
    static SensorState prev_sensor_state = SensorState::LIGHT_STATIONARY;
    static bool light_transition_handled = false;
    unsigned long now = millis();
    if (buttonPressed) {
        buttonPressed = false;
        if (now - last_recorded_button_press > BUTTON_DELAY) {
            character_selected = (character_selected + 1) % NUMBER_OF_CHARACTERS;
            last_recorded_button_press = now;
            Serial.printf("Button count: %d\n", character_selected);
        }
    }
    if (now - playback_start_time >= current_track_length) {
        SensorState current_sensor_state = get_sensor_state();
        ActivityResult activity_result = get_activity(current_sensor_state, prev_sensor_state, light_transition_handled);
        prev_sensor_state = current_sensor_state;
        light_transition_handled = activity_result.light_transition_handled;
        current_track_length = play_random_track(character_selected, activity_result.activity);
        playback_start_time = now;
    }
}

SensorState get_sensor_state() {
  bool light = is_it_light();
  bool moving = is_it_moving();
  if (light && moving) return SensorState::LIGHT_MOVING;
  if (!light && moving) return SensorState::DARK_MOVING;
  if (light && !moving) return SensorState::LIGHT_STATIONARY;
  return SensorState::DARK_STATIONARY;
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


ActivityResult get_activity(SensorState current, SensorState previous, bool light_transition_handled) {
    ActivityResult result;
    // JUST_REMOVED: first light after dark
    if (!light_transition_handled &&
        previous == SensorState::DARK_STATIONARY &&
        (current == SensorState::LIGHT_STATIONARY || current == SensorState::LIGHT_MOVING)) 
    {
        result.activity = JUST_REMOVED;
        result.light_transition_handled = true;  // mark that transition handled
        return result;
    }
    // Normal mappings
    switch (current) {
        case SensorState::LIGHT_STATIONARY:
            result.activity = DOING_NOTHING;
            break;
        case SensorState::LIGHT_MOVING:
            result.activity = PUTTING_ON;
            break;
        case SensorState::DARK_STATIONARY:
            result.activity = WORN_STANDING_STILL;
            break;
        case SensorState::DARK_MOVING:
            result.activity = WALKING;
            break;
    }
    result.light_transition_handled = light_transition_handled;
    return result;
}


unsigned long play_random_track(uint8_t character, Activity activity) {
    uint8_t activity_index = static_cast<uint8_t>(activity);
    StateTracks &state_tracks = track_table[character][activity_index];
    uint8_t track_idx = random(state_tracks.number_of_tracks);
    Track &selected_track = state_tracks.tracks[track_idx];
    Serial.printf("Playing character %d, activity %d, track %d, length %d\n",
              character, activity_index, selected_track.track_number, selected_track.length_ms);
    myDFPlayer.play(selected_track.track_number);
    return selected_track.length_ms;
}







