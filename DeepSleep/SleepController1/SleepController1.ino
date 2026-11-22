/*
Deep Sleep experiments, mainly for ESP32. This is the remote controller program
running on an ESP8266. 
Use this program in conjunction with Sleeper2.ino on the ESP32.

This ESP8266 sends an active LOW pulse through its D4 to pin GPIO 13 of ESP32 to put it into sleep;
another active LOW pulse through D5 pin to GPIO 12 of ESP32 wakes it up from sleep.
No level shifting is required since both are 3.3V devices.

RTC GPIOs of ESP32 for Deep Sleep Wake: 
On classic ESP32 (like ESP32-WROOM) boards, esp_sleep_enable_ext0_wakeup() 
requires an RTC GPIO (such as GPIO33, GPIO32, GPIO25, GPIO26, GPIO27, GPIO14, 
GPIO12, GPIO13, GPIO4, GPIO2, GPIO0). 
If you are using internal pull-downs, avoid input-only pins (GPIO34–GPIO39).
These pins lack internal pull-ups/pull-downs; Use external resistors.
*/

#include <Arduino.h>

const int SLEEP_PIN  = D4;
const int WAKEUP_PIN = D5;

void setup(){
  Serial.begin(115200);
  pinMode(SLEEP_PIN,  OUTPUT);
  digitalWrite(SLEEP_PIN, HIGH);  // the ESP32 input pins are active LOW
  pinMode(WAKEUP_PIN,  OUTPUT);
  digitalWrite(WAKEUP_PIN, HIGH); 
  Serial.println("\nDeep sleep Controller on ESP8266 begins..");
  Serial.println("Type s to put the ESP32 to sleep; type w to wake it up.");
}

int count = 0;
void loop(){
  if (!Serial.available()) return;
  String input = Serial.readStringUntil('\n');
  if(input.startsWith("s")) {
    Serial.println("Putting ESP32 to sleep..");
    digitalWrite(SLEEP_PIN, LOW);
    delay(200); // ESP32 is just polling this pin 
    digitalWrite(SLEEP_PIN, HIGH);
  } else if (input.startsWith("w")) {
    Serial.println("Waking up ESP32..");
    digitalWrite(WAKEUP_PIN, LOW);
    delay(200);
    digitalWrite(WAKEUP_PIN, HIGH);
  }
} 