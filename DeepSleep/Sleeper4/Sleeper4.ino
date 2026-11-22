/*
Deep Sleep experiments, mainly for ESP32.
Code by Gemini.
Use this program in conjunction with SleepController1.ino on the ESP8266.
An ESP8266 (or Arduino AVR with voltage dividers) gives a pulse to this ESP32 
pin GPIO 13 to put it into DEEP sleep; a pulse on GPIO 4 wakes it up from sleep.
The control pulses are active LOW.

New in this version: 
(1) Deep sleep mode. (The earlier 3 programs used light sleep).  
(2) The ESP32 connets to Wifi and starts a web server. Both are suspended during deep sleep. 
(3) Important parameters are stored in RTC memory to persist during deep sleeps.
(4) Simulates sensor data: Enter + or - on the serial terminal to increment/decrement
    the simulated value; The page is auto-refreshed once in 3 seconds
(5) enter  http://ip-address/state   to get a JSON API response

RTC GPIOs for Deep Sleep Wake: 
On classic ESP32 (like ESP32-WROOM) boards, esp_sleep_enable_ext0_wakeup() 
requires an RTC GPIO (such as GPIO33, GPIO32, GPIO25, GPIO26, GPIO27, GPIO14, 
GPIO12, GPIO13, GPIO4, GPIO2, GPIO0). These pins are monitored by the RTC even in deep sleep.
But waking up from light sleep can use many more general purpose GPIO pins.

Caution: GPIO 0,2,5,12, and 15 are boot strapping pins on ESOP32.
If you are using internal pull-downs, avoid input-only pins (GPIO34–GPIO39).
These pins lack internal pull-ups/pull-downs; Use external resistors.
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "esp_sleep.h"
#include "Secret.h"

const char* WIFI_SSID = ssid;  // from Secret.h
const char* WIFI_PASS = passwd;

const gpio_num_t WAKEUP_PIN = GPIO_NUM_4;   // RTC GPIO for EXT0 wake (active LOW)
const int SLEEP_PIN = 13;                   // Input from AVR: LOW = go to deep sleep
const int ESP_READY_PIN = 26;               // Optional output signal to 8266 controller: HIGH when ready

WebServer server(80);

// Retained across deep sleep in RTC slow memory
RTC_DATA_ATTR int latestSensorValue = 90;
RTC_DATA_ATTR uint32_t sampleCount = 0;
RTC_DATA_ATTR uint32_t sleepCycles = 0;
RTC_DATA_ATTR uint32_t wakeCount = 0;
RTC_DATA_ATTR char lastStatus[32] = "BOOT";

char rxLine[64];
size_t rxIdx = 0;

void handleRoot() {
  char html[900];
  snprintf(
      html,
      sizeof(html),
      "<!doctype html><html><head><meta charset='utf-8'>"
      "<meta name='viewport' content='width=device-width,initial-scale=1'>"
      "<meta http-equiv='refresh' content='3'>"
      "<title>AVR + ESP32 Node</title>"
      "<style>body{font-family:sans-serif;background:#111;color:#eee;padding:24px;}"
      ".card{background:#3d3d2d;border:1px solid #333;border-radius:16px;padding:16px;max-width:420px;}"
      ".val{font-size:32px;font-weight:bold;color:#4fd1c5;}</style></head>"
      "<body><div class='card'><h2>Sensor Dashboard</h2>"
      "<div class='val'>%d</div>"
      "<p>Samples: %lu</p>"
      "<p>Wakeups: %lu | Sleeps: %lu</p>"
      "<p>State: %s</p></div></body></html>",
      latestSensorValue,
      (unsigned long)sampleCount,
      (unsigned long)wakeCount,
      (unsigned long)sleepCycles,
      lastStatus);
  server.send(200, "text/html", html);
}

void handleApiState() {
  char json[192];
  snprintf(
      json,
      sizeof(json),
      "{\"value\":%d,\"samples\":%lu,\"wakes\":%lu,\"sleeps\":%lu,\"status\":\"%s\"}",
      latestSensorValue,
      (unsigned long)sampleCount,
      (unsigned long)wakeCount,
      (unsigned long)sleepCycles,
      lastStatus);
  server.send(200, "application/json", json);
}

// Connect to WiFi and start web server
void startNetworkAndDashboard() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Connecting to WiFi...");
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startAttempt < 10000)) {
    delay(100);
  }
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to WiFi.");
    return;
  }
  Serial.println("\nConnected.");
  server.on("/", handleRoot);
  server.on("/state", handleApiState);
  server.begin();
  snprintf(lastStatus, sizeof(lastStatus), "ACTIVE");
  // signal to 8266 that we are now awake and ready to receive commands
  digitalWrite(ESP_READY_PIN, LOW);  // this is a level signal, not a pulse
  Serial.print("Running web server at ");
  Serial.println(WiFi.localIP());
}

void enterDeepSleep() {
  Serial.println("Received sleep command!");
  snprintf(lastStatus, sizeof(lastStatus), "SLEEPING");
  sleepCycles++;
  digitalWrite(ESP_READY_PIN, HIGH); // tell thr 8266 Controller that we are off to sleep
  server.stop();
  Serial.println("Web server stopped");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("WiFi switched off");
  Serial.println("\nGoing to sleep - zzzzzZZZZ....\n");
  Serial.flush();
  delay(100);
  // Ensure wake pin starts HIGH before arming EXT0 active-low wakeup
  pinMode(WAKEUP_PIN, INPUT_PULLUP);
  esp_sleep_enable_ext0_wakeup(WAKEUP_PIN, LOW);
  esp_deep_sleep_start();  // this loses data in RAM, so we retain some parameters in RTC memory
}

// Send + or - characters through serial port to simulate changing sensor data
void readSimulatedSerialData() {
  String input = Serial.readStringUntil('\n');
  if(input.startsWith("+")) 
    latestSensorValue += 10;
  else if(input.startsWith("-")) 
    latestSensorValue -= 5;        
  sampleCount++;
  Serial.printf("Sensor data: %d  |  Sample count: %d\n", latestSensorValue, sampleCount);
}

void setup() {
  Serial.begin(115200);
  pinMode(SLEEP_PIN, INPUT_PULLUP);
  pinMode(WAKEUP_PIN, INPUT_PULLUP);
  pinMode(ESP_READY_PIN, OUTPUT);
  // optional: signal the 8266 that we are awake and ready to take commands
  digitalWrite(ESP_READY_PIN, LOW); // active low; this is a level signal, not a pulse
  delay(100);
  Serial.println("\nSetup: ESP32 Deep Sleep Tester starts...");
  
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  if (cause == ESP_SLEEP_WAKEUP_EXT0) {
    wakeCount++;
  }
  Serial.printf("Wake count = %d\n", wakeCount);
  startNetworkAndDashboard(); // connect to Wifi and start web server
}

void loop() {
  server.handleClient();
  if (Serial.available()) 
    readSimulatedSerialData(); // this simulated data is persisted in RTC memory
  if(digitalRead(SLEEP_PIN)) // active low
    return;
  delay(10); // denoise 8266 output pin?
  if(!digitalRead(SLEEP_PIN))
    enterDeepSleep();
}