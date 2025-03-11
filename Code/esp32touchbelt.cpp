#include <Arduino.h>
#include <BleKeyboard.h>

// BLE Keyboard Initialization
BleKeyboard bleKeyboard;

// Gesture input pins
#define CMD_PIN0 23 // 23
#define CMD_PIN1 22 // 22
#define CMD_PIN2 21 // 21
#define CMD_PIN3 19 // 19
#define CMD_PIN4 18 // 18

// Button input pins
#define BTN_HOME 26
#define BTN_APP_SWITCHER 27
#define BTN_CONTROL_CENTER 32
#define BTN_ROTOR 33

// VoiceOver modifier keys
#define VO_CTRL KEY_LEFT_CTRL
#define VO_ALT KEY_LEFT_ALT

// Command codes
const uint8_t CMD_NONE       = 0b00000;
const uint8_t DOUBLE_CLICK   = 0b00001;
const uint8_t MOVE_LEFT      = 0b00010;
const uint8_t MOVE_RIGHT     = 0b00011;
const uint8_t MOVE_UP        = 0b00100;
const uint8_t MOVE_DOWN      = 0b00101;
const uint8_t SINGLE_CLICK   = 0b00110;

// Maximum fingers supported
const uint8_t MAX_FINGERS = 3;

// Debounce delay
const unsigned long debounceDelay = 10;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Keyboard...");
  bleKeyboard.begin();

  // Configure gesture pins as input with pull-down resistors
  pinMode(CMD_PIN0, INPUT_PULLDOWN);
  pinMode(CMD_PIN1, INPUT_PULLDOWN);
  pinMode(CMD_PIN2, INPUT_PULLDOWN);
  pinMode(CMD_PIN3, INPUT_PULLDOWN);
  pinMode(CMD_PIN4, INPUT_PULLDOWN);

  // Configure button pins as input with pull-down resistors
  pinMode(BTN_HOME, INPUT_PULLUP);
  pinMode(BTN_APP_SWITCHER, INPUT_PULLUP);
  pinMode(BTN_CONTROL_CENTER, INPUT_PULLUP);
  pinMode(BTN_ROTOR, INPUT_PULLUP);
}

void loop() {
  static unsigned long lastReadTime = 0;
  static uint8_t stableCmd = CMD_NONE;
  static bool commandChanged = false;

  if (bleKeyboard.isConnected()) {
    // Read gesture pin states
    uint8_t cmd = 0;
    cmd |= digitalRead(CMD_PIN0) << 0;
    cmd |= digitalRead(CMD_PIN1) << 1;
    cmd |= digitalRead(CMD_PIN2) << 2;
    cmd |= digitalRead(CMD_PIN3) << 3;
    cmd |= digitalRead(CMD_PIN4) << 4;

    // Detect command changes
    if (cmd != stableCmd) {
      commandChanged = true;
      lastReadTime = millis();
      stableCmd = cmd;
    }

    if (commandChanged && (millis() - lastReadTime >= debounceDelay)) {
      commandChanged = false;

      if (stableCmd != CMD_NONE) {
        uint8_t fingerIndex = (stableCmd >> 3) & 0b11;
        uint8_t fingerCount = fingerIndex + 1;
        if (fingerCount > MAX_FINGERS) {
          fingerCount = MAX_FINGERS;
        }

        uint8_t eventCode = stableCmd & 0b00111;
        handleGesture(fingerCount, eventCode);
      }
    }

    handleButtons();
  }

  delay(1);
}

void handleGesture(uint8_t fingerCount, uint8_t eventCode) {
  Serial.print("getUniqueKey - fingerCount: ");
  Serial.print(fingerCount);
  Serial.print(", eventCode: ");
  Serial.println(eventCode);
  String actionType = "";

  // Map gestures to keyboard shortcuts
  switch (eventCode) {
    case DOUBLE_CLICK:
      actionType = "Double Click";
      sendGestureCommand(fingerCount, DOUBLE_CLICK);
      break;
    case MOVE_LEFT:
      actionType = "Move Left";
      sendGestureCommand(fingerCount, MOVE_LEFT);
      break;
    case MOVE_RIGHT:
      actionType = "Move Right";
      sendGestureCommand(fingerCount, MOVE_RIGHT);
      break;
    case MOVE_UP:
      actionType = "Move Up";
      sendGestureCommand(fingerCount, MOVE_UP);
      break;
    case MOVE_DOWN:
      actionType = "Move Down";
      sendGestureCommand(fingerCount, MOVE_DOWN);
      break;
    case SINGLE_CLICK:
      actionType = "Single Click";
      sendGestureCommand(fingerCount, SINGLE_CLICK);
      break;
    default:
      actionType = "Unknown";
  }

  Serial.print("Action: ");
  Serial.print(fingerCount);
  Serial.print("-Finger ");
  Serial.println(actionType);
}

void sendGestureCommand(uint8_t fingerCount, uint8_t eventCode) {
  // Determine unique key for the combination of fingerCount and eventCode
  char key = getUniqueKey(fingerCount, eventCode);

  // Send the key using BLE keyboard
  bleKeyboard.press(VO_CTRL);
  bleKeyboard.press(VO_ALT);
  bleKeyboard.write(key);
  bleKeyboard.releaseAll();
}

char getUniqueKey(uint8_t fingerCount, uint8_t eventCode) {
  // Map each finger count and event code to a unique key
  // Example: ASCII codes for keys 'A' to 'Z', 'a' to 'z', etc.
  Serial.print("eventCode:");
  Serial.println(eventCode);
  switch (fingerCount) {
    case 1:
      switch (eventCode) {
        case DOUBLE_CLICK: return 'a';
        case MOVE_LEFT:    return 'b';
        case MOVE_RIGHT:   return 'c';
        case MOVE_UP:      return 'd';
        case MOVE_DOWN:    return 'e';
        case SINGLE_CLICK: return 'f';
      }
      break;
    case 2:
      switch (eventCode) {
        case DOUBLE_CLICK: return 'g';
        case MOVE_LEFT:    return 'h';
        case MOVE_RIGHT:   return 'i';
        case MOVE_UP:      return 'j';
        case MOVE_DOWN:    return 'k';
        case SINGLE_CLICK: return 'l';
      }
      break;
    case 3:
      switch (eventCode) {
        case DOUBLE_CLICK: return 'm';
        case MOVE_LEFT:    return 'n';
        case MOVE_RIGHT:   return 'o';
        case MOVE_UP:      return 'p';
        case MOVE_DOWN:    return 'q';
        case SINGLE_CLICK: return 'r';
      }
      break;
  }
  return 'z'; // Default fallback key if no match
}

// Variables to store the previous states of each button
bool prevHomeState = HIGH;
bool prevAppSwitcherState = HIGH;
bool prevControlCenterState = HIGH;
bool prevRotorState = HIGH;

void handleButtons() {
  // Read the current state of each button
  bool homeState = digitalRead(BTN_HOME);
  bool appSwitcherState = digitalRead(BTN_APP_SWITCHER);
  bool controlCenterState = digitalRead(BTN_CONTROL_CENTER);
  bool rotorState = digitalRead(BTN_ROTOR);

  // Detect transition from HIGH to LOW for each button
  if (homeState == LOW && prevHomeState == HIGH) {
    Serial.println("Action: Home (Command + H)");
    bleKeyboard.press(KEY_LEFT_GUI);
    bleKeyboard.write('h');
    bleKeyboard.releaseAll();
  }

  if (appSwitcherState == LOW && prevAppSwitcherState == HIGH) {
    Serial.println("Action: App Switcher (Command + Up Arrow)");
    bleKeyboard.press(KEY_LEFT_GUI);
    bleKeyboard.write(KEY_UP_ARROW);
    bleKeyboard.releaseAll();
  }

  if (controlCenterState == LOW && prevControlCenterState == HIGH) {
    Serial.println("Action: Control Center (Command + C)");
    bleKeyboard.press(KEY_LEFT_GUI);
    bleKeyboard.write('c');
    bleKeyboard.releaseAll();
  }

  if (rotorState == LOW && prevRotorState == HIGH) {
    Serial.println("Action: Rotor Switch (VO + Command + Right Arrow)");
    bleKeyboard.press(VO_CTRL);
    bleKeyboard.press(VO_ALT);
    bleKeyboard.press(KEY_LEFT_GUI);
    bleKeyboard.write(KEY_RIGHT_ARROW);
    bleKeyboard.releaseAll();
  }

  // Update the previous states with the current states
  prevHomeState = homeState;
  prevAppSwitcherState = appSwitcherState;
  prevControlCenterState = controlCenterState;
  prevRotorState = rotorState;
}
