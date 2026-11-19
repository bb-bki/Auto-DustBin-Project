#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Servo.h>

constexpr uint8_t TRIG_PIN = 3;
constexpr uint8_t ECHO_PIN = 1;
constexpr uint8_t REDD_PIN = 6;
constexpr uint8_t GREE_PIN = 8;
constexpr uint8_t SERV_PIN = 5;

constexpr uint8_t THRESHOLD_CM = 20; // This is the distance from sensor in cm to open lid

Adafruit_NeoPixel strip(1, 10, NEO_GRB + NEO_KHZ800); // 1 LED on GPIO10

Servo servo;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, Auto Dust Bin v3!");

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(REDD_PIN, OUTPUT);
  pinMode(GREE_PIN, OUTPUT);
  servo.attach(SERV_PIN);

  strip.begin();
  strip.setBrightness(30);
  strip.show();
  strip.setPixelColor(0, strip.Color(0,255,0)); // RED
  strip.show();
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
    servo.write(104);
    digitalWrite(REDD_PIN, LOW);
    digitalWrite(GREE_PIN, HIGH);
    Serial.println("Bin open");
  }

  delay(1000); // Wait 1s
}
