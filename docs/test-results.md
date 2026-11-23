# Smart Automatic Dustbin — Lab Test Results

## 1. Test Environment & Hardware Setup
* **Controller:** ESP32-C3 (`115200` baud Serial Monitor)
* **Sensor & Logic Level:** HC-SR04 (`TRIG` on `GPIO 3`, `ECHO` via level shifter to `GPIO 1`)
* **Actuator:** SG90 Servo (`0°` closed, `90°` open on `GPIO 5`)
* **Status LEDs:** Red LED (`GPIO 6` — closed/idle), Green LED (`GPIO 8` — open)
* **Power:** External `5V` supply with common `GND` across controller, sensor, level shifter, and servo
* **Tester(s) & Date:** [Fill in names and lab date]

---

## 2. Functional & Hysteresis Test Cases

| Test ID | Scenario / Distance | Expected Lid, LED & Serial Behavior | Actual Result | Pass / Fail | Notes |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **TC-01** | **Startup / Idle (no object)** | Lid at `0°` (closed), Red LED ON, Green LED OFF; distance prints cleanly or times out to `999 cm` without triggering open. | | | |
| **TC-02** | **Approach within open threshold ($\le 20\text{ cm}$, e.g., `~18 cm`)** | Lid moves to `90°` (open), Green LED ON, Red LED OFF; Serial logs state transition to open. | | | |
| **TC-03** | **Hysteresis band (`20 cm` to `30 cm`, e.g., `~25 cm`)** | No rapid opening/closing chatter; lid holds its current state smoothly without fluttering. | | | |
| **TC-04** | **Move past close threshold ($\ge 30\text{ cm}$, e.g., `~35 cm`)** | Lid returns to `0°` (closed), Red LED ON, Green LED OFF; Serial logs state transition to closed. | | | |
| **TC-05** | **Sustained presence ($\le 20\text{ cm}$)** | Lid stays open (`90°`) and Green LED stays ON while object remains within open range. | | | |

---

## 3. Edge Cases & Stability Checks

| Test ID | Edge Case | What to Watch For | Actual Result | Pass / Fail | Notes |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **EC-01** | **No echo / `30 ms` timeout (`999 cm`)** | `pulseIn()` times out cleanly without hanging `loop()`; check whether `999 cm` causes an unintended close or is handled safely. | | | |
| **EC-02** | **Non-blocking timing (`millis()`)** | Sampling runs on interval (e.g., `250 ms`) without freezing Serial output or state checks. | | | |
| **EC-03** | **Servo load & brownout check** | Moving between `0°` and `90°` does not reset the ESP32-C3 or cause LEDs/Serial to drop out. | | | |

---

## 4. Serial Monitor Sample Log (`115200` baud)
```text
[Paste representative Serial Monitor output from your lab run here]