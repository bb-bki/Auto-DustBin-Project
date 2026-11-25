# **HC-SR04**
## 1. Ultrasonic Sensor Timing and Signal Constraints

|Event | Explanation|
|--------|----------|
| **Trigger Requirements:** |The HC-SR04 requires a minimum 10-microsecond (10us) HIGH pulse on the *TRIG* pin to initiate a measurement burst (sending out an 8-cycle sonic burst at 40KHz).|
|**Speed of sound calculation**|Sound travels at approximately 0.0343cm/µs in ambient air. Because the sound wave travels to the object and back, the total duration must be divided by 2:|

> ### Distance(cm) = (Echo_Duration (µs) x 0.0343) / 2

|Event|Explanation|
|----|----|
|**Timeout and edge Cases**| If no echo returns (e.g. sound disperses or no object is present), waiting indefinitely will block the program. Implementing a **30-millisecond** (30,000µs) Timeout via pulsein() caps the maximum measurable distance at roughly 510cm. If the pulse times out, the firmware returns a designated error/out-of-range value of **999cm**|
|**Logic level voltage protection**| The sensor's **Echo** pin outputs ***5V*** Logic, while the ESP32-C3 operates strictly on ***3.3V*** Logic. Direct connections risk damaging the microcontroller pin, necessitating a logic-level converter (or voltage divider circuit) on GPIO 1.|

## 2. SG90 Servo Motor & PWM Actuation
|Event|Explanation|
|----|----|
|**PWM Specifications**|Standard Hobby servos like the SG90 expect a 50Hz PWM signal (a 20ms period frame). Pulse widths typically range from 1.0ms (0 degrees) to 2.0ms (180 degrees).|
|**Operational angles for Dustbin**| - ***0 degree***: Bin lid fully closed (rest position). - ***90 degree***: Bin lid fully open (clears the disposal opening.)|
|**Power Isolation**| Servos draw current spikes under load. Powering the SG90 directly from the ESP32-C3's 3.3V and 5V pin can cause brownout and microcontroller reboots. It requires an independent ***5V power supply*** sharing a **common ground** with the ESP32-C3.|

## 3. Hysteresis Algorithmic Design

|Event|Explanation|
|----|----|
|**The chattering problem**| Without a buffer zone, an object hovering right at a single threshold (e.g. exactly 20cm), causes rapid sensor jitter, forcing the servo and LEDs to flutter incessantly.|
|**Dual-Threshold solution**| - ***Opening Threshold (<=20cm):*** Forces the system state to transition from LID_CLOSED to LID_OPEN. - ***Closing Threshold (>=30cm):*** Requires the target to move past a wider outer boundary before triggering LID_CLOSED.|
|**The Hysteresis Band (10cm Buffer)**|Creates a stable operational deadzone that completely eliminates relay/servo chattering under fluctuating noise.|

## 4. Non-Blocking Architecture (millis())
|Event|Explanation|
|----|----|
|**Avoiding** ***delay()***|Using a blocking delay pauses the entire CPU, freezing Serial prints, LED updates, and responsiveness.|
|**Tick-Based Sampling**|The firmware utilises ***millis()*** to check the ultrasonic sensor at regular non-blocking intervals (e.g. every 250ms), keeping processor free to handle state transitions smoothly.|

---
<br>
<br>

# **Subsystem Design Notes: SG90 Servo Motor Actuation**
## 1. Core Operating Principles & PWM Specifications
|Event|Explanation|
|----|----|
|**Signal Requirements:** |Standard hobby servos like the SG90 are driven via **Pulse Width Modulation (PWM)** at a standard frequency of **50Hz** (meaning a complete pulse frame repeats every 20ms)|
| **Pulse Width Mapping:** |The angular position of the servo shaft is determined by the duration of the HIGH pulse within that 20ms frame:|

- 1.0ms pulse corresponds to **0°** (fully closed).
- 1.5ms pulse corresponds to **90°** (midpoint).
- 2.0ms pulse corresponds to **180°** (fully open).

|Event|Explanation|
|----|----|
| **Application Mapping for the Dustbin:**| *0°* The resting home position where the lid is completely closed. <br> *90°* The active position where the lid rotates up and clears the disposal opening.<br> *We* intentionally limit the maximum angle to 90° so the servo arm doesn't over-rotate or put unnecessary mechanical strain on the plastic chassis hinges.|

## 2. Pin Mapping & Hardware Integration
|Event|Explanation|
|----|----|
|**Control Signal:** | Connected directly from **GPIO 5** on the ESP32-C3 to the servo's PWM signal line (usually orange or white wire).|
|**Power & Ground Considerations:**|The SG90 operates on **5V** (red wire) and returns via ground (brown/black wire). *Critical Rule:* Never power a servo directly from the microcontroller's 3.3V logic line or rely entirely on the board's internal regulator for heavy motor current. Under load or during sudden movements, servo motors draw high current spikes that can cause voltage dropouts, triggering random brownout resets on the ESP32-C3. Always ensure the external 5V power supply shares a **common ground** with the ESP32-C3 GND pin.|

## 3. Firmware Logic & Non-Blocking Execution
|Event|Explanation|
|-----|----|
|**Library Integration:**| Utilizing the ESP32Servo library abstracts away manual timer calculations, allowing clean commands like lidServo.write(90) and lidServo.write(0).|
|**Preventing Stalling / Jitter:**| Once the servo reaches its target angle, holding the PWM signal active continuously provides holding torque, but updating it too frequently inside a fast loop can cause unnecessary chatter.<br><br> State checks ensure that lidServo.write() is only called *once* when a state transition actually occurs (i.e., when transitioning from LID_CLOSED to LID_OPEN), rather than flooding the servo driver every single cycle.|

---
<br>
<br>

# **Subsystem Design Notes: ESP32-C3 (Controller)**
## 1. What It Does & How It Works
|Event|Explanation|
|----|----|
|**Main Control Loop:**| The ESP32-C3 acts as the central controller: it triggers ultrasonic distance measurements, reads the returned echo pulse, applies the opening/closing hysteresis logic, and updates the servo position and status LEDs.|
|**How Signals Are Handled:**| It drives **GPIO 3** (`TRIG`) as an output to start each ping, reads **GPIO 1** (`ECHO`) as an input after stepping the signal down from **5V** to **3.3V** logic, sends PWM angle commands on **GPIO 5** to the SG90 servo, and switches **GPIO 6** (red LED) and **GPIO 8** (green LED).|
|**Serial Monitoring:**| Sends distance readings and lid state changes over Serial at **115200 baud** so the team can observe real-time behavior and debug edge cases.|

## 2. State Decisions & Safety Behavior
|Event|Explanation|
|----|----|
|**Threshold & Hysteresis Decisions:**| Compares measured distance against the open and close thresholds (such as opening within **20 cm** and staying open until moving to **30 cm** or farther) instead of flipping on a single noisy reading.|
|**Handling No-Echo & Obstructions:**| When no echo returns within **30 ms** (`999 cm`), reliable firmware avoids slamming the lid shut abruptly—requiring consecutive clear readings, a closing delay (such as waiting after the hand leaves), and stopping or reopening if an obstruction is detected[cite: 4].|

---
<br>
<br>

# **Subsystem Design Notes: Status LEDs (Green / Red)**
## 1. What They Do & How They Work
|Event|Explanation|
|----|----|
|**Red LED (Closed State):**| Connected to **GPIO 6**; the ESP32-C3 sets this pin HIGH when the lid is closed (including at power-on) so users get a clear visual cue that the bin is shut and idle[cite: 4].|
|**Green LED (Open State):**| Connected to **GPIO 8**; the ESP32-C3 sets this pin HIGH when the lid opens (hand detected) so users know the bin is open and ready for waste[cite: 4].|
|**How Switching Works:**| Whenever the lid transitions between open and closed, the firmware turns one LED ON and the other LED OFF at the same time so the two indicators never conflict.|

## 2. Electrical Protection & Circuit Details
|Event|Explanation|
|----|----|
|**Current-Limiting Resistors:**| Each LED's anode connects through a resistor (such as **150 Ω**) to its GPIO pin while the cathode connects to **GND**, limiting current flow to a safe level for the LED and microcontroller pin.|

---
<br>
<br>

# **Subsystem Design Notes: 5V Power Supply**
## 1. What It Does & How It Works
|Event|Explanation|
|----|----|
|**Powering the 5V Devices:**| Supplies **5V** power to the **SG90 servo motor** and **HC-SR04 ultrasonic sensor** (and feeds the **5V** high-voltage side of the level shifter) so those components get the voltage they require to operate.|
|**How It Supports Smooth Operation:**| Because servo movement creates brief current spikes, a dedicated **5V** power source helps keep voltage steady rather than straining the controller's regulator.|

## 2. Shared Ground & Voltage Reference
|Event|Explanation|
|----|----|
|**Common Ground Reference:**| The **5V** power supply, ESP32-C3, HC-SR04, SG90, and level shifter all tie to a **common GND** (shared ground)[cite: 4]. Without a shared ground, voltage measurements and PWM signals lack a common reference point, causing erratic readings or servo jitter.|
|**Safe Level Shifting:**| By pairing the **5V** side with the ESP32-C3's **3.3V** logic side across the level shifter, **5V** echo pulses are safely adapted to **3.3V** without over-voltaging **GPIO 1**.|


---

## 6. Firmware Bench Testing Notes (Lab Session)

### Pre-Power Checklist
- [ ] **GPIO 1 (`ECHO`):** Confirmed `5 V` echo line goes through the logic-level converter (`HV` at `5 V`, `LV` at `3.3 V`) before reaching **GPIO 1**.
- [ ] **GPIO 5 (`SERVO1 SIG`):** Connected to SG90 signal wire; SG90 powered from external **5 V** supply, not directly from a **3.3 V** pin.
- [ ] **GPIO 6 & GPIO 8 (LEDs):** Red LED (`GPIO 6`) and green LED (`GPIO 8`) wired with current-limiting resistors (`150 Ω`).
- [ ] **Common Ground:** ESP32-C3, **5 V** power supply, HC-SR04, SG90, and level shifter all share a common `GND`.

### Bench Observations & Serial Log (`115200` baud)

| Test Case / Check | What Happened on the Bench | Notes / Fixes Needed |
| :--- | :--- | :--- |
| **Power-on / Idle (no object)** | | |
| **Hand within `20 cm` (e.g., `18 cm`)** | | |
| **Hand in hysteresis zone (e.g., `25 cm`)** | | |
| **Hand at `30 cm` or farther** | | |
| **Object held within `20 cm`** | | |
| **No echo / timeout (`30 ms` $\rightarrow$ `999 cm`)** | | |
| **Servo movement (`0°` to `90°`) & power stability** | | |

<!-- Old workings*
# **HC-SR04**
## 1. Ultrasonic Sensor Timing and Signal Constraints

|Event | explanation|
|--------|----------|
| **Trigger Requirements:** |The HC-SR04 requires a minimum 10-microsecond (10us) HIGH pulse on the *TRIG* pin to initiate a measurement burst (sending out an 8 second-cycle sonic burst at 40KHz).|
|**Speed of sound calculation**|Sound travels at approximately 0.343cm/µs in ambient air. Because the sound wave travels to the object and back, the total duration must be divided by 2: |

> ### Distance(cm) = (Echo_Duration (µs) x 0.0343) /  2

|Event|Explanation|
|----|----|
|**Timeout and edge Cases**| If no echo returns(e.g sound disperses or no object is present), waiting indefinitely will block the program. Implementing a **30-millisecond** (30,000µs) Timeout via pulsein() caps the maximum measurebale distance at roughly 510cm. If the pulse times out, the firmware returns a designated error/out-of-range value of **999cm**|
|**Logic level voltage protection**| The sensor's **Echo** pin outputs ***5V*** Logic, while the ESP32-C3 operates strictly on ***3.3V*** Logic. Direct connections risk damaging the microcontroller pin, necessitating a logic-level converter (or voltage divided circuit) on GPIO 1.|

## 2. SG90 Servo Motor & PWM Actuation
|Event|Explanation|
|----|----|
|**PWM Specificiations**|Standard Hobby servos like the SG90 expect a 50Hz PWM signal (a 20ms period frame). Plus widths typically range from 1.0ms(0 degrees) to 2.0ms (180 degrees).|
|**Operational angles for Dustbin**| - ***0 degree***: Bin lid fully closed (rest position). - ***90 degrees***: Bin lid fully open (clears the disposal opening.)|
|**Power Isolation**| Servos draw current spikes under load. Powering the SG90 directly from the ESP32-C3's 3.3V and 5V pin can cause brownout and microcontroller reboots. It requires an independent ***5V power supply*** sharing a **common ground** with the ESP32-C3.|

## 3. Hysteresis Algorithmic Design

|Event|Explanation|
|----|----|
|**The chattering problem**| Without a buffer zone, an object hovering right at a single thresholed(e.g exactly 20cm), causes rapid sensor jitter, forcing the servo and LEDs to flutter incessantly.|
|**Dual-Threshold solution**| - ***Opening Threshold(<=20cm):*** Forces the system state to transition from LID_CLOSED to LID_OPEN. - ***Closing Threshold(>=30cm):*** Requires the target to move past a wider outer boundary before triggering LID_Closed.|
|**The Hysteresis Band(10cm Buffer)**|Creates a stable operational deadzone that completely eliminates relay/servo chattering under fluctuating noise.|

## 4. Non-Blocking Architecture(millis())
|Event|Explanation|
|----|----|
|**Avoiding** ***delay()***|Using a blocking delay pauses the entire GPU, freezing Serial prints, LED updates, and responsiveness.|
|**Tick-Based Sampling**|The firmware utilises ***millis()***| to check the ultrasonic sensor at regular non-blocking intervals (e.g...every 250ms), keeping processor free to handle state transitions smoothly.\
---
<br>
<br>

# **Subsystem Design Notes: SG90 Servo Motor Actuation**
## 1. Core Operating Principles & PWM Specifications
|Event|Explanation|
|----|----|
|**Signal Requirements:** |Standard hobby servos like the SG90 are driven via **Pulse Width Modulation (PWM)** at a standard frequency of **50Hz** (meaning a complete pulse frame repeats every 20ms)|
| **Pulse Width Mapping:** |The angular position of the servo shaft is determined by the duration of the HIGH pulse within that 20ms frame:|

  - 1.0ms pulse corresponds to **0°** (fully closed).
  - 1.5ms pulse corresponds to **90°** (midpoint).
  - 2.0ms pulse corresponds to **180°** (fully open).

|event|explanation|
|----|----|
| **Application Mapping for the Dustbin:**| *0°* The resting home position where the lid is completely closed. <br> *90°* The active position where the lid rotates up and clears the disposal opening.<br> *We* intentionally limit the maximum angle to 90° so the servo arm doesn't over-rotate or put unnecessary mechanical strain on the plastic chassis hinges.|

## 2. Pin Mapping & Hardware Integration
|event|explanation|
|----|----|
   **Control Signal:** | Connected directly from **GPIO 5** on the ESP32-C3 to the servo's PWM signal line (usually orange or white wire).|
  |**Power & Ground Considerations:**|The SG90 operates on **5V** (red wire) and returns via ground (brown/black wire). *Critical Rule:* Never power a servo directly from the microcontroller's 3.3V logic line or rely entirely on the board's internal regulator for heavy motor current. Under load or during sudden movements, servo motors draw high current spikes that can cause voltage dropouts, triggering random brownout resets on the ESP32-C3. Always ensure the external 5V power supply shares a **common ground** with the ESP32-C3 GND pin.|

## 3. Firmware Logic & Non-Blocking Execution
|Event|Explanation|
|-----|----|
 |**Library Integration:**| Utilizing the ESP32Servo library abstracts away manual timer calculations, allowing clean commands like lidServo.write(90) and lidServo.write(0).|
| **Preventing Stalling / Jitter:**| Once the servo reaches its target angle, holding the PWM signal active continuously provides holding torque, but updating it too frequently inside a fast loop can cause unnecessary chatter.<br><br> State checks ensure that lidServo.write() is only called *once* when a state transition actually occurs (i.e., when transitioning from LID_CLOSED to LID_OPEN), rather than flooding the servo driver every single cycle.|
---
<br>
<br>
-->