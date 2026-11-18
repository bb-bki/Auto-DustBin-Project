# HC-SR04
## Ultrasonic Sensor Timing and Signal Constraints

|Event | explanation|
|--------|----------|
| **Trigger Requirements:** |The HC-SR04 requires a minimum 10-microsecond (10us) HIGH pulse on the *TRIG* pin to initiate a measurement burst (sending out an 8 second-cycle sonic burst at 40KHz).|
|**Speed of sound calculation**|Sound travels at approximately 0.343cm/µs in ambient air. Because the sound wave travels to the object and back, the total duration must be divided by 2: |

> # Distance(cm) = (Echo_Duration (µs) x 0.0343) /  2

|Event|Explanation|
|----|----|
|**Timeout and edge Cases**| If no echo returns(e.g sound disperses or no object is present), waiting indefinitely will block the program. Implementing a **30-millisecond** (30,000µs) Timeout via pulsein() caps the maximum measurebale distance at roughly 510cm. If the pulse times out, the firmware returns a designated error/out-of-range value of **999cm**|
|**Logic level voltage protection**| The sensor's **Echo** pin outputs ***5V*** Logic, while the ESP32-C3 operates strictly on ***3.3V*** Logic. Direct connections risk damaging the microcontroller pin, necessitating a logic-level converter (or voltage divided circuit) on GPIO 1.|
|||

