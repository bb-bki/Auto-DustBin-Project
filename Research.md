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
* **Control Signal:** Connected directly from **GPIO 5** on the ESP32-C3 to the servo's PWM signal line (usually orange or white wire).
* **Power & Ground Considerations:**
  * The SG90 operates on **5V** (red wire) and returns via ground (brown/black wire).
  * *Critical Rule:* Never power a servo directly from the microcontroller's 3.3V logic line or rely entirely on the board's internal regulator for heavy motor current. Under load or during sudden movements, servo motors draw high current spikes that can cause voltage dropouts, triggering random brownout resets on the ESP32-C3.
  * Always ensure the external 5V power supply shares a **common ground** with the ESP32-C3 GND pin.

## 3. Firmware Logic & Non-Blocking Execution
|Event|Explanation|
|-----|----|
 |**Library Integration:**| Utilizing the ESP32Servo library abstracts away manual timer calculations, allowing clean commands like lidServo.write(90) and lidServo.write(0).|
| **Preventing Stalling / Jitter:**| Once the servo reaches its target angle, holding the PWM signal active continuously provides holding torque, but updating it too frequently inside a fast loop can cause unnecessary chatter.<br><br> State checks ensure that lidServo.write() is only called *once* when a state transition actually occurs (i.e., when transitioning from LID_CLOSED to LID_OPEN), rather than flooding the servo driver every single cycle.|
---
<br>
<br>