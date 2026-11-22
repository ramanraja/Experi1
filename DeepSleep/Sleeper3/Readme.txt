A second controller is introduced to control the sleep/wakup state of the ESP32.

The ESP32 receives two signals from another 8266 board:
 - GPIO13 (active low) to go to sleep mode
 - GPIO4 (active low) to wakeup
Use this program in conjuction with SleepController1.ino on the 8266.

New in this version: It connects to Wifi and starts its own web server.
The web server sleeps when the 8266 Sleep Mode Controller signals it.
Wifi is also shut down to conserve battery.
Upon wakeup, it reconnects to WiFi and starts the web server again.

Designed by Gemini.