/*
Deep Sleep experiments, mainly for ESP32.
Code by Gemini.
Use this program in conjunction with SleepController1.ino on the ESP8266.
An ESP8266 (or Arduino AVR with voltage dividers) gives a pulse to this ESP32 
pin GPIO13 to put it into sleep; a pulse on GPIO 4 wakes it up from sleep.
The control pulses are active LOW.

New in this version: It uses a 1-second Timer instead of delay(1000) in the main loop. 
Reason: The ESP8266 SleepController asserts its sleep pin for less than 1 second, then it is lost in polling!

RTC GPIOs for Deep Sleep Wake: 
On classic ESP32 (like ESP32-WROOM) boards, esp_sleep_enable_ext0_wakeup() 
requires an RTC GPIO (such as GPIO33, GPIO32, GPIO25, GPIO26, GPIO27, GPIO14, 
GPIO12, GPIO13, GPIO4, GPIO2, GPIO0). 

Caution: GPIO 0,2,5,12, and 15 are boot strapping pins on ESOP32.
If you are using internal pull-downs, avoid input-only pins (GPIO34–GPIO39).
These pins lack internal pull-ups/pull-downs; Use external resistors.
*/

#include <Arduino.h>
#include <WiFi.h>
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "Timer.h"

const gpio_num_t SLEEP_PIN  = GPIO_NUM_13;    // this is an enum
const gpio_num_t WAKEUP_PIN = GPIO_NUM_4; 
Timer T;

void setup(){
  Serial.begin(115200);
  Serial.println("\nDeep sleep experiment begins..");
  pinMode(SLEEP_PIN,  INPUT_PULLUP);
  pinMode(WAKEUP_PIN, INPUT_PULLUP);
  T.every (1000, bloc);
}

void loop(){
  T.update();
  if(digitalRead(SLEEP_PIN)) // active low
    return;
  delay(10); // denoise 8266 output pin?
  if(!digitalRead(SLEEP_PIN))
    go_to_light_sleep();
}

int count = 0;
void bloc() {
  Serial.print("Elapsed seconds: ");
  Serial.println(count);
  count++;
}

void go_to_light_sleep(){
  Serial.println("Going to light sleep. Let the SleepController wake me up.");
  // optional:
  //WiFi.disconnect(true);
  //WiFi.mode(WIFI_OFF);

  // Configure GPIO wake up pin (active LOW)
  gpio_wakeup_enable(WAKEUP_PIN, GPIO_INTR_LOW_LEVEL);  // Aliter: GPIO_INTR_HIGH_LEVEL
  esp_sleep_enable_gpio_wakeup();

  Serial.println("\nzzzzzzzzZZZZ........");
  Serial.flush();
  delay(500);  // the 8266 Controller assets the SLEEP_PIN for 200 mSec; so avoid retriggering

  // Blocks here until WAKEUP_PIN goes LOW:
  esp_light_sleep_start();  

  // Aliter: deep sleep, loses all state 
  //esp_deep_sleep_start();

  // Back from sleep; all varaible states are preserved
  Serial.printf("\nGood morning! (We had paused at %d)\n", count-1);

  // Disable GPIO wakeup feature, so normal pin use is clean
  gpio_wakeup_disable(WAKEUP_PIN);
}
