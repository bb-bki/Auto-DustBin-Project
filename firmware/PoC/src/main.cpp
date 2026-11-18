#include <Arduino.h>
#include <Servo.h>

constexpr uint8_t TRIG_PIN = 2;
constexpr uint8_t ECHO_PIN = 3;
constexpr uint8_t REDD_PIN = 12;
constexpr uint8_t GREE_PIN = 13;
constexpr uint8_t SERV_PIN = 22;

constexpr uint8_t THRESHOLD_CM = 20; // This is the distance from sensor in cm to open lid

Servo servo;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, Bin PoC v2!");

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(REDD_PIN, OUTPUT);
  pinMode(GREE_PIN, OUTPUT);
  servo.attach(SERV_PIN);

  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(TRIG_PIN, HIGH);
  delay(10);
  digitalWrite(TRIG_PIN, LOW);

  int distance = pulseIn(ECHO_PIN, HIGH);
  distance = distance / 58; // Converts raw input to centimetres

  if (distance == 0) { // This means the sensor is not working or connected.
    servo.write(0);
    digitalWrite(REDD_PIN, LOW);
    digitalWrite(GREE_PIN, LOW);
    Serial.println("ERROR");
  } else if (distance > 20) { // Close the bin
    servo.write(0);
    digitalWrite(REDD_PIN, HIGH);
    digitalWrite(GREE_PIN, LOW);
    Serial.println("Bin closed");
  } else if (distance <= 20) { // Open the bin
    servo.write(90);
    digitalWrite(REDD_PIN, LOW);
    digitalWrite(GREE_PIN, HIGH);
    Serial.println("Bin open");
  }

  delay(1000); // Wait 1s
}
