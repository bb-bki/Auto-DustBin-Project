# HC-SR04
## Ultrasonic Sensor Timing and Signal Constraints

|Event | explanation|
|--------|----------|
| **Trigger Requirements:** |The HC-SR04 requires a minimum 10-microsecond (10us) HIGH pulse on the *TRIG* pin to initiate a measurement burst (sending out an 8 second-cycle sonic burst at 40KHz).|
|**Speed of sound calculation**|Sound travels at approximately 0.343cm/µs in ambient air. Because the sound wave travels to the object and back, the total duration must be divided by 2: |

> ### Distance(cm) = (Echo_Duration (µs) x 0.0343) /  2

|Event|Explanation|
|----|----|
|**Timeout and edge Cases**| If no echo returns(e.g sound disperses or no object is present), waiting indefinitely will block the program. Implementing a **30-millisecond** (30,000µs) Timeout via pulsein() caps the maximum measurebale distance at roughly 510cm. If the pulse times out, the firmware returns a designated error/out-of-range value of **999cm**|
|**Logic level voltage protection**| The sensor's **Echo** pin outputs ***5V*** Logic, while the ESP32-C3 operates strictly on ***3.3V*** Logic. Direct connections risk damaging the microcontroller pin, necessitating a logic-level converter (or voltage divided circuit) on GPIO 1.|

## SG90 Servo Motor & PWM Actuation
|Event|Explanation|
|----|----|
|**PWM Specificiations**|Standard Hobby servos like the SG90 expect a 50Hz PWM signal (a 20ms period frame). Plus widths typically range from 1.0ms(0 degrees) to 2.0ms (180 degrees).|
|**Operational angles for Dustbin**| - ***0 degree***: Bin lid fully closed (rest position). - ***90 degrees***: Bin lid fully open (clears the disposal opening.)|
|**Power Isolation**| Servos draw current spikes under load. Powering the SG90 directly from the ESP32-C3's 3.3V and 5V pin can cause brownout and microcontroller reboots. It requires an independent ***5V power supply*** sharing a **common ground** with the ESP32-C3.|

## Hysteresis Algorithmic Design

|Event|Explanation|
|----|----|
|**The chattering problem**| Without a buffer zone, an object hovering right at a single thresholed(e.g exactly 20cm), causes rapid sensor jitter, forcing the servo and LEDs to flutter incessantly.|
|**Dual-Threshold solution**| - ***Opening Threshold(<=20cm):*** Forces the system state to transition from LID_CLOSED to LID_OPEN. - ***Closing Threshold(>=30cm):*** Requires the target to move past a wider outer boundary before triggering LID_Closed.|
|||

