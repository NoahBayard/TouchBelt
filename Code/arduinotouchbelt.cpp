#include <PS2Mouse.h>

// Define PS/2 pins
#define MOUSE_DATA 5
#define MOUSE_CLOCK 4

PS2Mouse mouse(MOUSE_CLOCK, MOUSE_DATA, STREAM);

// Function to read the next available byte (blocking)
uint8_t waitForByte() {
  return mouse.read();  // Blocks until a byte is received
}

// Global variables to store capabilities
bool isCapExtended = false;
bool isCapMultiFinger = false;
bool isCapPalmDetect = false;
uint8_t currentWmode = 0;

// Function to extract infoMajor from Model ID
// For Model ID 0x0 0x09 0x0, assume infoMajor=4 based on infoSensor=9
uint8_t extractInfoMajor(uint8_t modelID1, uint8_t modelID2, uint8_t modelID3) {
  if ((modelID1 == 0x0) && (modelID2 == 0x09) && (modelID3 == 0x0)) {
    return 4;  // Assume infoMajor=4 for TW41xx230 Wide pad module
  }
  // Add more conditions for other models if necessary
  return 0;
}

// Function to query touchpad status and return the status bytes
void queryStatus(uint8_t &status1, uint8_t &status2, uint8_t &status3) {
  mouse.write(0xE9);            // Request Status
  uint8_t ack = waitForByte();  // Expect ACK
  Serial.print("ACK for Request Status: 0x");
  Serial.println(ack, HEX);

  // Read the 3-byte status response
  status1 = waitForByte();
  status2 = waitForByte();
  status3 = waitForByte();

  Serial.print("Status1: 0x");
  Serial.print(status1, HEX);
  Serial.print(", Status2: 0x");
  Serial.print(status2, HEX);
  Serial.print(", Status3: 0x");
  Serial.println(status3, HEX);
}

// Function to query current touchpad modes (Request 01)
void queryModes() {
  Serial.println("Querying current touchpad modes...");

  uint8_t status1, status2, status3;
  queryStatus(status1, status2, status3);  // Reuse the unified query function

  // Log the mode details
  Serial.print("Current Mode - Status1: 0x");
  Serial.print(status1, HEX);
  Serial.print(", Status2: 0x");
  Serial.print(status2, HEX);
  Serial.print(", Status3: 0x");
  Serial.println(status3, HEX);
}

// Function to query the Model ID
void queryModelID() {
  Serial.println("Querying Model ID...");

  // Send the Identify command
  mouse.write(0xF2);

  // Read ACK (0xFA)
  uint8_t ack = waitForByte();
  Serial.print("ACK for Identify Command: 0x");
  Serial.println(ack, HEX);

  // Read 3-byte Model ID
  uint8_t modelID1 = waitForByte();
  uint8_t modelID2 = waitForByte();
  uint8_t modelID3 = waitForByte();

  Serial.print("Model ID: 0x");
  Serial.print(modelID1, HEX);
  Serial.print(" 0x");
  Serial.print(modelID2, HEX);
  Serial.print(" 0x");
  Serial.println(modelID3, HEX);

  // Extract infoMajor from Model ID
  uint8_t infoMajor = extractInfoMajor(modelID1, modelID2, modelID3);
  Serial.print("infoMajor Version: ");
  Serial.println(infoMajor);

  if (infoMajor >= 4) {
    Serial.println("infoMajor >= 4: Assuming extended capabilities are supported based on model ID.");
    isCapExtended = true;
    isCapMultiFinger = true;
    isCapPalmDetect = true;
  } else {
    Serial.println("infoMajor < 4: Extended capabilities are not supported.");
    // However, based on modelID=0x0 0x09 0x0, assume extended capabilities are supported
    if ((modelID1 == 0x0) && (modelID2 == 0x09) && (modelID3 == 0x0)) {
      isCapExtended = true;
      isCapMultiFinger = true;
      isCapPalmDetect = true;
      Serial.println("Assuming extended capabilities are supported based on model ID.");
    } else {
      isCapExtended = false;
      isCapMultiFinger = false;
      isCapPalmDetect = false;
    }
  }
}

void queryAndSetWmode(uint8_t desiredWmode) {
  Serial.println("Querying and setting Wmode...");

  // Query current status to check Wmode
  mouse.write(0xE9);            // Request Status
  uint8_t ack = waitForByte();  // ACK
  Serial.print("ACK for Request Status: 0x");
  Serial.println(ack, HEX);

  uint8_t status1 = waitForByte();
  uint8_t status2 = waitForByte();
  uint8_t status3 = waitForByte();

  // Extract Wmode bit from status2
  uint8_t currentWmodeBit = status2 & 0x01;  // Wmode bit
  Serial.print("Current Wmode: ");
  Serial.println(currentWmodeBit);

  // Only attempt to change Wmode if necessary
  if (currentWmodeBit != desiredWmode) {
    Serial.print("Setting Wmode to: ");
    Serial.println(desiredWmode);

    // Encode mode byte for Wmode = desiredWmode
    // The exact sequence depends on the touchpad's protocol; adjust as necessary
    mouse.write(0xE8);
    mouse.write(0x03);
    waitForByte();  // Example: Bits 7-6: Absolute mode, Wmode enabled
    mouse.write(0xE8);
    mouse.write(0x00);
    waitForByte();  // Bits 5-4: Reserved, set to 0
    mouse.write(0xE8);
    mouse.write(0x00);
    waitForByte();  // Bits 3-2: Reserved, set to 0
    mouse.write(0xE8);
    mouse.write(desiredWmode & 0x01);
    waitForByte();  // Bit 1: Wmode on/off

    // Finalize Wmode setting with F3 14
    mouse.write(0xF3);
    mouse.write(0x14);  // Set Sample Rate to confirm
    waitForByte();      // ACK for F3
    waitForByte();      // ACK for 0x14

    // Verify Wmode
    mouse.write(0xE9);    // Request Status again
    ack = waitForByte();  // ACK
    status1 = waitForByte();
    status2 = waitForByte();
    status3 = waitForByte();
    currentWmodeBit = status2 & 0x01;

    Serial.print("Wmode after setting: ");
    Serial.println(currentWmodeBit);

    if (currentWmodeBit == desiredWmode) {
      Serial.println("Wmode successfully updated.");
    } else {
      Serial.println("Failed to update Wmode.");
    }
  } else {
    Serial.println("Wmode is already set to the desired value.");
  }
}

// Function to set a new mode byte with additional resolution and sampling setup
void setMode(uint8_t mode) {
  Serial.print("Setting new mode to: 0x");
  Serial.println(mode, HEX);

  // Step 1: Send "Set Resolution" command with required values
  const uint8_t resolutions[] = { 0x00, 0x01, 0x02 };  // Resolutions for X, Y, Z axes
  for (int i = 0; i < 3; i++) {
    mouse.write(0xE8);             // Set Resolution command
    uint8_t ack1 = waitForByte();  // ACK for E8
    mouse.write(resolutions[i]);   // Send resolution value
    uint8_t ack2 = waitForByte();  // ACK for resolution value
    Serial.print("Set Resolution: ");
    Serial.print(resolutions[i], HEX);
    Serial.print(", ACK1: 0x");
    Serial.print(ack1, HEX);
    Serial.print(", ACK2: 0x");
    Serial.println(ack2, HEX);
  }

  // Step 2: Encode the mode byte into four E8 commands
  mouse.write(0xE8);
  mouse.write((mode >> 6) & 0x03);
  waitForByte();  // Bits 7 and 6
  mouse.write(0xE8);
  mouse.write((mode >> 4) & 0x03);
  waitForByte();  // Bits 5 and 4
  mouse.write(0xE8);
  mouse.write((mode >> 2) & 0x03);
  waitForByte();  // Bits 3 and 2
  mouse.write(0xE8);
  mouse.write(mode & 0x03);
  waitForByte();  // Bits 1 and 0

  // Step 3: Finalize the mode setting with Set Sample Rate
  mouse.write(0xF3);             // Set Sample Rate command
  uint8_t ack3 = waitForByte();  // ACK for F3
  mouse.write(0x14);             // Sample Rate 20 (as per absolute mode requirements)
  uint8_t ack4 = waitForByte();  // ACK for 0x14
  Serial.print("Set Sample Rate: 20, ACK3: 0x");
  Serial.print(ack3, HEX);
  Serial.print(", ACK4: 0x");
  Serial.println(ack4, HEX);

  Serial.println("New mode applied.");
}

// Function to verify mode change
void verifyModeChange() {
  Serial.println("Verifying mode change...");
  queryModes();  // Re-query the modes to confirm the change
}

// Function to perform the Synaptics "magic knock"
void performMagicKnock() {
  Serial.println("Performing Synaptics magic knock sequence...");

  // Send E8 00 four times to prepare for advanced mode
  for (int i = 0; i < 4; i++) {
    mouse.write(0xE8);
    mouse.write(0x00);
    uint8_t ack = waitForByte();  // ACK
    Serial.print("Sent 0xE8 0x00, ACK: 0x");
    Serial.println(ack, HEX);
  }

  // Add the sample rate sequence as part of the magic knock
  const uint8_t sampleRates[] = { 200, 100, 80 };  // Required sample rates
  for (int i = 0; i < 3; i++) {
    mouse.write(0xF3);             // Set Sample Rate command
    uint8_t ack1 = waitForByte();  // ACK for F3
    mouse.write(sampleRates[i]);   // Send sample rate value
    uint8_t ack2 = waitForByte();  // ACK for sample rate value
    Serial.print("Set Sample Rate: ");
    Serial.print(sampleRates[i]);
    Serial.print(", ACK1: 0x");
    Serial.print(ack1, HEX);
    Serial.print(", ACK2: 0x");
    Serial.println(ack2, HEX);
  }
}

// Function to disable data reporting
void disableDataReporting() {
  Serial.println("Disabling data reporting...");
  mouse.write(0xF5);            // Command to disable data reporting
  uint8_t ack = waitForByte();  // Expect 0xFA
  Serial.print("ACK from 0xF5: 0x");
  Serial.println(ack, HEX);
}

// Function to enable data reporting
void enableDataReporting() {
  Serial.println("Enabling data reporting...");
  mouse.write(0xF4);            // Command to enable data reporting
  uint8_t ack = waitForByte();  // Expect 0xFA
  Serial.print("ACK from 0xF4: 0x");
  Serial.println(ack, HEX);
}

// Define enums for directions and event types
enum Direction {
  NONE,
  LEFT,
  RIGHT,
  UP,
  DOWN
};

// Define event codes ensuring SINGLE_CLICK is not 0
const uint8_t SINGLE_CLICK = 0b110;  // 7
const uint8_t DOUBLE_CLICK = 0b001;  // 1
const uint8_t MOVE_LEFT = 0b010;     // 2
const uint8_t MOVE_RIGHT = 0b011;    // 3
const uint8_t MOVE_UP = 0b100;       // 4
const uint8_t MOVE_DOWN = 0b101;     // 5

// CMD_NONE remains as 0b00000 (0)
const uint8_t CMD_NONE = 0b00000;  // 0

// Define command pins (Digital Pins 13, 12, 11, 10, 9)
const uint8_t commandPins[5] = { 0, 1, 2, 3, 7 };

// Function to convert direction to event type
uint8_t getEventCode(const char *direction) {
  if (strcmp(direction, "Left") == 0) {
    return MOVE_LEFT;
  } else if (strcmp(direction, "Right") == 0) {
    return MOVE_RIGHT;
  } else if (strcmp(direction, "Up") == 0) {
    return MOVE_UP;
  } else if (strcmp(direction, "Down") == 0) {
    return MOVE_DOWN;
  } else {
    return SINGLE_CLICK;
  }
}

// Function to send command via commandPins with serial prints
void sendEncodedCommand(uint8_t cmd, unsigned long holdDuration = 15, bool printPins = true) {  // holdDuration in milliseconds
  if (printPins) {
    Serial.print("Sending cmd: ");
    Serial.println(cmd);

    // Set each pin based on the corresponding bit in cmd
    for (int i = 0; i < 5; i++) {
      bool state = (cmd >> i) & 0x01;
      digitalWrite(commandPins[i], state ? HIGH : LOW);

      // Print the state of each pin
      Serial.print("Pin ");
      Serial.print(commandPins[i]);
      Serial.print(": ");
      Serial.println(state ? "HIGH" : "LOW");
    }

    // Hold the command for the specified duration to ensure detection
    delay(holdDuration);

    // Reset the command pins to LOW
    for (int i = 0; i < 5; i++) {
      digitalWrite(commandPins[i], LOW);

      // Print the reset state of each pin
      Serial.print("Reset Pin ");
      Serial.print(commandPins[i]);
      Serial.println(": LOW");
    }
  } else {
    // Set each pin based on the corresponding bit in cmd without printing
    for (int i = 0; i < 5; i++) {
      bool state = (cmd >> i) & 0x01;
      digitalWrite(commandPins[i], state ? HIGH : LOW);
    }

    // Print only the cmd value
    Serial.print("cmd");
    Serial.println(cmd);

    // Hold the command for the specified duration
    delay(holdDuration);

    // Reset the command pins to LOW without printing
    for (int i = 0; i < 5; i++) {
      digitalWrite(commandPins[i], LOW);
    }
  }
}

void setup() {
  Serial.begin(115200);
  mouse.initialize();
  Serial.println("Mouse initialized in stream mode.");

  // Step 1: Perform the Synaptics "magic knock"
  performMagicKnock();

  // Step 2: Query current modes
  queryModes();

  // Step 3: Query Model ID and determine capabilities
  queryModelID();

  // Step 4: Query and set Wmode
  queryAndSetWmode(1);  // Set Wmode to 1 if not already set

  // Step 5: Disable data reporting
  disableDataReporting();

  // Step 6: Set Absolute Mode (0x8A as an example)
  setMode(0x8A);

  // Step 7: Verify the mode change
  verifyModeChange();

  // Step 8: Enable data reporting
  enableDataReporting();

  // Initialize command pins as outputs
  for (int i = 0; i < 5; i++) {
    pinMode(commandPins[i], OUTPUT);
    digitalWrite(commandPins[i], LOW);  // Set all command pins to LOW initially
  }

  Serial.println("Configuration complete.");
}

void loop() {
  // 1) Read 6 bytes from Synaptics (Absolute mode)
  uint8_t packet[6];
  for (int i = 0; i < 6; i++) {
    packet[i] = waitForByte();
  }

  // 2) Extract fields
  uint8_t b1 = packet[0];
  uint8_t b2 = packet[1];
  uint8_t b3 = packet[2];  // Z
  uint8_t b4 = packet[3];
  uint8_t b5 = packet[4];
  uint8_t b6 = packet[5];

  uint8_t statusByte = b1;

  // Decompose X, Y
  uint8_t Y12 = (b4 >> 5) & 0x01;
  uint8_t X12 = (b4 >> 4) & 0x01;
  uint8_t xHi4 = (b2 & 0x0F);
  uint8_t yHi4 = (b2 >> 4) & 0x0F;
  uint16_t rawX = ((uint16_t)X12 << 12) | ((uint16_t)xHi4 << 8) | b5;
  uint16_t rawY = ((uint16_t)Y12 << 12) | ((uint16_t)yHi4 << 8) | b6;

  // Z
  uint8_t Z = b3;

  // W
  uint8_t w32 = (b1 >> 4) & 0x03;
  uint8_t w1 = (b1 >> 2) & 0x01;
  uint8_t w0 = (b4 >> 2) & 0x01;
  uint8_t W = (w32 << 2) | (w1 << 1) | w0;

  // Only handle if status is {0x80, 0x90} and W is in {0, 1, 4}
  bool validStatus = ((statusByte == 0x80) || (statusByte == 0x90));
  bool validW = ((W == 0) || (W == 1) || (W == 4));
  if (!validStatus || !validW) {
    return;
  }

  // Determine fingerCount based on status and W
  uint8_t fingerCount = 0;
  if ((statusByte == 0x80) && (Z < 15)) {
    fingerCount = 0;  // idle
  } else if ((statusByte == 0x90) && (W == 4) && (Z > 15)) {
    fingerCount = 1;
  } else if ((statusByte == 0x80) && (W == 0) && (Z > 15)) {
    fingerCount = 2;
  } else if ((statusByte == 0x80) && (W == 1) && (Z > 15)) {
    fingerCount = 3;
  }
  // else => 0 fallback

  // 3) Compute relative deltas
  static uint16_t oldX = 0, oldY = 0;
  int16_t diffX = (int16_t)rawX - (int16_t)oldX;
  int16_t diffY = (int16_t)rawY - (int16_t)oldY;
  oldX = rawX;
  oldY = rawY;

  static const float ST = 0.2f;
  float scaledDX = ST * (float)diffX;
  float scaledDY = ST * (float)diffY;
  int16_t dX = (int16_t)scaledDX;
  int16_t dY = (int16_t)scaledDY;

  // If movement deltas are too large, consider it as noise and ignore
  if ((abs(dX) > 200) || (abs(dY) > 200)) {
    return;
  }

  // 4) Movement and Click Detection
  static bool movementInProgress = false;
  static unsigned int freq1 = 0, freq2 = 0, freq3 = 0;
  static int32_t sumDX = 0, sumDY = 0;
  static unsigned long movementStartTime = 0;

  // Override for 3-finger
  static bool hasSeen3 = false;

  // Variables for single click detection
  static bool pendingSingleClick = false;
  static unsigned long pendingClickTime = 0;
  static uint8_t pendingClickCount = 0;
  const unsigned long DOUBLE_CLICK_MS = 250;  // Threshold for double click
  const unsigned long CLICK_TIME_MS = 90;     // Maximum duration for a click (ms)

  // Variable to track if an event was handled in this loop
  bool eventHandled = false;

  // Handle movement start
  if ((fingerCount > 0) && !movementInProgress) {
    // Movement has started
    movementInProgress = true;
    freq1 = freq2 = freq3 = 0;
    sumDX = 0;
    sumDY = 0;
    movementStartTime = millis();

    // Reset the 3-finger override
    hasSeen3 = false;
  }
  // Handle movement end
  else if ((fingerCount == 0) && movementInProgress) {
    // Movement has ended
    movementInProgress = false;
    unsigned long movementEndTime = millis();
    unsigned long duration = movementEndTime - movementStartTime;

    // Determine finger count at the end of movement
    uint8_t finalCount = 0;
    if (hasSeen3) {
      finalCount = 3;
    } else {
      if (freq1 >= freq2) finalCount = 1;
      else finalCount = 2;
    }

    // Determine direction based on accumulated deltas
    const char *direction = "None";
    if (abs(sumDX) > abs(sumDY)) {
      // Horizontal movement on the trackpad (translating to vertical movement in our application)
      if (sumDX > 50) direction = "Down";
      else if (sumDX < -50) direction = "Up";
    } else {
      // Vertical movement on the trackpad (translating to horizontal movement in our application)
      if (sumDY < -50) direction = "Left";
      else if (sumDY > 50) direction = "Right";
    }

    // Check if the movement duration is less than CLICK_TIME_MS (90 ms)
    bool isClick = false;
    if ((finalCount >= 1) && (finalCount <= 3)) {
      if (duration < CLICK_TIME_MS) {
        isClick = true;
      }
    }

    // Handle Click or Movement based on duration
    if (isClick || (strcmp(direction, "None") == 0 && finalCount > 0)) {
      // Click Handling
      if (pendingSingleClick && (millis() - pendingClickTime < DOUBLE_CLICK_MS) && (pendingClickCount == finalCount)) {
        // Double Click detected
        Serial.print("** DOUBLE CLICK with ");
        Serial.print(finalCount);
        Serial.println("-finger! **");

        // Encode and send double click command
        uint8_t fingerIndex = finalCount - 1;             // 0-based index
        uint8_t cmd = (fingerIndex << 3) | DOUBLE_CLICK;  // DOUBLE_CLICK = 0b001

        // Send the encoded command via digital pins 13,12,11,10,9 without printing pin states
        sendEncodedCommand(cmd, 15, true);  // holdDuration=2ms, printPins=true

        eventHandled = true;

        // Reset pending click
        pendingSingleClick = false;
      } else {
        // Register this click as pending
        pendingSingleClick = true;
        pendingClickTime = millis();
        pendingClickCount = finalCount;
      }

    } else if (finalCount > 0) {
      // Movement occurred
      Serial.print("Movement Detected: Direction=");
      Serial.print(direction);
      Serial.print(", Finger Count=");
      Serial.println(finalCount);

      // Encode and send movement command
      uint8_t fingerIndex = finalCount - 1;  // 0-based index
      uint8_t eventCode = getEventCode(direction);

      uint8_t cmd = (fingerIndex << 3) | (eventCode & 0b111);

      // Send the encoded command via digital pins 13,12,11,10,9 and print pin states
      sendEncodedCommand(cmd, 15, true);  // holdDuration=2ms, printPins=true
      Serial.print("cmd");
      Serial.println(cmd);
      eventHandled = true;
    }
  }

  // If in a movement, accumulate finger counts and deltas
  if (movementInProgress) {
    if (fingerCount == 3) {
      hasSeen3 = true;
    } else if (fingerCount == 1) {
      freq1++;
    } else if (fingerCount == 2) {
      freq2++;
    }
    sumDX += dX;
    sumDY += dY;
  }

  // 5) Handle Pending Single Clicks
  if (pendingSingleClick) {
    if (millis() - pendingClickTime >= DOUBLE_CLICK_MS) {
      // Time elapsed without a second click; confirm single click
      Serial.print("** SINGLE CLICK with ");
      Serial.print(pendingClickCount);
      Serial.println("-finger! **");

      // Encode and send single click command
      uint8_t fingerIndex = pendingClickCount - 1;      // 0-based index
      uint8_t cmd = (fingerIndex << 3) | SINGLE_CLICK;  // SINGLE_CLICK = 0b110

      // Send the encoded command via digital pins 13,12,11,10,9 with pin states printed
      sendEncodedCommand(cmd, 15, true);  // holdDuration=2ms, printPins=true
      Serial.print("cmd");
      Serial.println(cmd);
      eventHandled = true;

      // Reset pending click
      pendingSingleClick = false;
    }
  }

  // 6) If no event was handled in this loop, set command pins to CMD_NONE (00000) without printing pin states
  if (!eventHandled) {
    sendEncodedCommand(CMD_NONE, 15, false);  // CMD_NONE = 0b00000, holdDuration=2ms, printPins=false
    Serial.print("cmd");
    Serial.println(CMD_NONE);
  }
}
