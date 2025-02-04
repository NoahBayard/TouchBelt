# TouchBelt: Assistive Touchpad for VoiceOver Navigation

A project to create a **wearable trackpad** and button-based controller that allows **blind users** to interact with their iPhone **without taking it out of their pocket**.

---

## 1. Project Overview

### Goal
- To create a **compact, wearable** input device that allows blind users to control **VoiceOver gestures** hands-free.
- The **touchpad** detects gestures (swipes, taps, multi-finger touches).
- The **ESP32 sends commands via Bluetooth**, using **VoiceOver keyboard shortcuts** to simulate iPhone gestures.

### How It Works
1. A **Synaptics touchpad** from an old laptop detects swipes, taps, and multi-finger gestures.
2. An **Arduino MKR** reads the gesture data from the touchpad.
3. The **ESP32 processes the gesture and sends it via Bluetooth** to an iPhone as a keyboard shortcut.
4. Additional **hardware buttons** are included for quick actions (Home, App Switcher, etc.).

---

## 2. Bill of Materials

| Component               | Description                                      |
|-------------------------|--------------------------------------------------|
| **Synaptics Touchpad**  | Extracted from an **HP/Lenovo laptop**          |
| **Arduino MKR**         | Reads touchpad gestures                         |
| **ESP32 (WROOM-32E)**  | Sends Bluetooth commands to the iPhone          |
| **Lithium Battery + TP4056** | Powers the system                         |
| **3D Printed Case**     | Holds the components                            |
| **Tactile Buttons (x4)** | Used for quick-access iPhone functions         |

---

## 3. Hardware Setup

### 3.1 Wiring

| Component  | Connected to |
|------------|-------------|
| **Touchpad Data (PS2)** | **Arduino MKR** |
| **Gesture Output (Digital Pins)** | **ESP32** |
| **ESP32 (Bluetooth Controller)** | **iPhone (BLE Keyboard)** |
| **Buttons (4)** | **ESP32 GPIO Pins** |

- **PS/2 Touchpad Wiring**
  - **Clock (T3) → Arduino MKR Pin 4**
  - **Data (T5) → Arduino MKR Pin 5**
  - **VCC (T20) → 5V**
  - **GND (T1) → GND**

- **Gesture Communication**
  - **Arduino MKR Digital Pins → ESP32 Digital Pins**
  - Encodes **5-bit gesture commands** (e.g., swipes, clicks)

- **Button Mapping (ESP32)**
  - **Home Button → GPIO 26**
  - **App Switcher → GPIO 27**
  - **Control Center → GPIO 32**
  - **Rotor → GPIO 33**

---

## 4. Software Setup

### 4.1 Arduino MKR Code (Touchpad Interface)
- Uses **PS2Mouse library** to read touch gestures.
- Decodes **multi-finger gestures** (single tap, double tap, swipe).
- Sends **5-bit encoded commands** to ESP32 via **digital pins**.

### 4.2 ESP32 Code (Bluetooth Keyboard)
- Uses **BleKeyboard library** to send iPhone VoiceOver shortcuts.
- Reads gesture commands from Arduino.
- Converts commands to **VoiceOver-compatible keyboard inputs**.

#### Example Mapping

| Gesture  | Sent Key (BLE Keyboard) |
|----------|-------------------------|
| 1-Finger Swipe Left  | **Ctrl + Alt + Left Arrow** |
| 1-Finger Swipe Right | **Ctrl + Alt + Right Arrow** |
| 1-Finger Swipe Up    | **Ctrl + Alt + Up Arrow** |
| 1-Finger Swipe Down  | **Ctrl + Alt + Down Arrow** |
| 2-Finger Tap        | **Ctrl + Alt + Space (Click)** |

---

## 5. Installation & Setup

### 5.1 Installing Required Libraries
1. Install **Arduino IDE**.
2. Install **ESP32 Board Package** (`https://dl.espressif.com/dl/package_esp32_index.json`).
3. Install the following libraries:
   - **PS2Mouse** (For reading touchpad)
   - **BleKeyboard** (For Bluetooth control)

### 5.2 Flashing the Code
1. **Flash the Arduino MKR** with the `Touchpad_Reader.ino` sketch.
2. **Flash the ESP32** with the `ESP32_BLE_Keyboard.ino` sketch.

### 5.3 Pairing with iPhone
1. Enable **VoiceOver** on iPhone.
2. Go to **Settings > Bluetooth**.
3. Pair with **ESP32 Keyboard**.

---

## 6. Testing the Device

### 6.1 Gesture Recognition
- **Check Serial Monitor** on the Arduino MKR to confirm it detects:
  - Swipe Left/Right/Up/Down
  - Single/Double Tap
  - Multi-Finger Gestures

### 6.2 Bluetooth Command Execution
- Check that the **iPhone responds correctly** to VoiceOver shortcuts.

---

## 7. Troubleshooting

### 7.1 Common Issues

| Issue | Solution |
|-------|----------|
| Touchpad not responding | Check PS/2 connections, ensure Absolute Mode is enabled |
| ESP32 not detected by iPhone | Reflash firmware, check `BleKeyboard.begin()` |
| Buttons not working | Ensure pull-up resistors are enabled |

---

## 8. Future Improvements
- **Better latency testing** for real-time feedback.  
- **Gesture customization via app** for user-specific needs.  
- **More durable 3D printed enclosure** for everyday use.  

---

## 9. Conclusion

The **TouchBelt project** demonstrates that it is possible to create **an alternative smartphone input method** for blind users using **recycled laptop touchpads, an Arduino, and an ESP32 BLE keyboard emulator**. This device allows **hands-free navigation**, making **on-the-go smartphone use** much more accessible.

---

## 10. License

This project is released under the **MIT License**. See [LICENSE](LICENSE) for details.

