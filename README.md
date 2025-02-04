# TouchBelt
A project to create a wearable trackpad and button-based controller that allows blind users to interact with their iPhone without taking it out of their pocket.

## Project Overview

### Goal
To create a compact, wearable input device that allows blind users to control VoiceOver gestures hands-free.
The touchpad detects gestures (multifinger taps and swipes).
The ESP32 sends commands via Bluetooth, using VoiceOver keyboard shortcuts to simulate iPhone gestures.

### How It Works
A Synaptics touchpad from an old laptop detects swipes, taps, and multi-finger gestures.
An Arduino MKR reads the gesture data from the touchpad.
The ESP32 processes the gesture and sends it via Bluetooth to an iPhone as a keyboard shortcut.
Additional hardware buttons are included for quick actions (Home, App Switcher, etc.).

## Bill of Materials

Component	Description
Synaptics Touchpad	Extracted from an HP/Lenovo laptop
Arduino MKR	Reads touchpad gestures
ESP32 (WROOM-32E)	Sends Bluetooth commands to the iPhone
Lithium Battery + TP4056	Powers the system
3D Printed Case	Holds the components
Tactile Buttons (x4)	Used for quick-access iPhone functions

## Hardware Setup

Component	Connected to
Touchpad Data (PS2)	Arduino MKR
Gesture Output (Digital Pins)	ESP32
ESP32 (Bluetooth Controller)	iPhone (BLE Keyboard)
Buttons (4)	ESP32 GPIO Pins
PS/2 Touchpad Wiring
Clock (T3) → Arduino MKR Pin 4
Data (T5) → Arduino MKR Pin 5
VCC (T20) → 5V
GND (T1) → GND
Gesture Communication
Arduino MKR Digital Pins → ESP32 Digital Pins
Encodes 5-bit gesture commands (e.g., swipes, clicks)
Button Mapping (ESP32)
Home Button → GPIO 26
App Switcher → GPIO 27
Control Center → GPIO 32
Rotor → GPIO 33

## Software Setup

### Arduino MKR Code (Touchpad Interface)
Uses PS2Mouse library to read touch gestures.
Decodes multi-finger gestures (single tap, double tap, swipe).
Sends 5-bit encoded commands to ESP32 via digital pins.
### ESP32 Code (Bluetooth Keyboard)
Uses BleKeyboard library to send iPhone VoiceOver shortcuts.
Reads gesture commands from Arduino.
Converts commands to VoiceOver-compatible keyboard inputs.
Example Mapping:

Gesture	Sent Key (BLE Keyboard)
1-Finger Swipe Left	Ctrl + Alt + Left Arrow
1-Finger Swipe Right	Ctrl + Alt + Right Arrow
1-Finger Swipe Up	Ctrl + Alt + Up Arrow
1-Finger Swipe Down	Ctrl + Alt + Down Arrow
2-Finger Tap	Ctrl + Alt + Space (Click)

## Installation & Setup

### Installing Required Libraries
Install Arduino IDE.
Install ESP32 Board Package (https://dl.espressif.com/dl/package_esp32_index.json).
Install the following libraries:
PS2Mouse (For reading touchpad)
BleKeyboard (For Bluetooth control)

### Flashing the Code
Flash the Arduino MKR with the Touchpad_Reader.ino sketch.
Flash the ESP32 with the ESP32_BLE_Keyboard.ino sketch.

### Pairing with iPhone
Enable VoiceOver on iPhone.
Go to Settings > Bluetooth.
Pair with ESP32 Keyboard.


## Testing the Device

### Gesture Recognition
Check Serial Monitor on the Arduino MKR to confirm it detects:
Swipe Left/Right/Up/Down
Single/Double Tap
For 1, 2 and 3 fingers. 
Some touchpads might not support multitouch.
 

### Bluetooth Command Execution
Check that the iPhone responds correctly to VoiceOver shortcuts.

## Future Improvements

- Better latency testing for real-time feedback.
- Gesture customization via app for user-specific needs.
- More durable 3D printed enclosure for everyday use.

## Conclusion

The TouchBelt project demonstrates that it is possible to create an alternative smartphone input method for blind users using recycled laptop touchpads, an Arduino, and an ESP32 BLE keyboard emulator. This device allows hands-free navigation, making on-the-go smartphone use much more accessible. 
