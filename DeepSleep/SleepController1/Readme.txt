Sleep /Wakeup controller board:

Run this program on an ESP8266.
Connect D4 pin to GPIO13 of ESP32: sleep signal
Connect D5 to GPIO 4 of ESP32: wake up signal
(If, you are using an AVR like Arduino Uno instead of 8266, you need
1K:2K voltage dividers on both pins)

send signal "s" or "w" through the serial terminal to put the ESP32 to sleep/wakeup mode respectively.

Note: The ESP32 polls for the sleep pin, so the SLEEP signal has to be present for a longer time (100 milli second was found to work well). But the WAKEUP signal can be shorter, because it is an interrupt pulse.