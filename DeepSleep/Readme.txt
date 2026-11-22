Deep Sleep experiments, mainly for ESP32.
Code by Gemini.

An Arduino AVR or 8266 gives a pulse to an ESP32 to put it into sleep;
a pulse on another pin wakes it up from sleep.

RTC GPIOs for Deep Sleep Wake: 

On classic ESP32 (like ESP32-WROOM) boards, esp_sleep_enable_ext0_wakeup() 
requires an RTC GPIO (such as GPIO33, GPIO32, GPIO25, GPIO26, GPIO27, GPIO14, 
GPIO12, GPIO13, GPIO4, GPIO2, GPIO0). 

If you are using internal pull-downs, avoid input-only pins (GPIO34–GPIO39).
These pins lack internal pull-ups/pull-downs; Use external resistors.

The most convenient pin is of course, GPIO0 with internal pullup enabled.
Configure the wake up pulse active LOW. 
const gpio_num_t WAKEUP_PIN = GPIO_NUM_0;  // this is an enum
gpio_wakeup_enable(WAKEUP_PIN, GPIO_INTR_LOW_LEVEL);
-------------------------------------------------------------------------
Prompt:
I have an ESP32 and an AVR board like Arduino Uno in the same enclosure. The AVR is monitoring the real world through sensors, and the ESP32 does the processing. I want to put the ESP32 to sleep when there is a lull in real world activity as detected by the sensors.
Please give me a code snippet where the AVR board will assert a GPIO pin connected to the ESP32, and the ESP32 will go into sleep mode. To wake it up when data becomes available, the AVR asserts another output pin.
The ESP32 has to retain state when it wakes up. In particular, it will serve a web page dashboard that should maintain continuity and respond soon after waking up.
---------------------------------------------------------------------------
Deep sleep:

Dashboard Continuity Across Deep Sleep: Deep sleep powers down main SRAM and drops Wi-Fi connections. Store dashboard state (last sensor reading, sample counter, lull count, last wake reason) in RTC_DATA_ATTR so it survives deep sleep immediately upon wake, and re-start the WebServer right after Wi-Fi reconnects.
---------------------------------------------------------------------------
Prompt:
What is the difference between
esp_sleep_enable_ext0_wakeup() and 
gpio_wakeup_enable() ?

Gemini:

See also the XL sheet screen shot Deep Vs Light Sleep.png

Core Difference: Sleep Mode & Hardware Domain
esp_sleep_enable_ext0_wakeup() arms the RTC IO controller to wake the ESP32 from deep sleep (it also works in light sleep) using one single RTC GPIO triggered by a specific logic level (0 = LOW, 1 = HIGH). 
Because main digital/APB domains power down in deep sleep, only RTC-domain pins can participate.

gpio_wakeup_enable() arms the digital/RTC GPIO wakeup logic for light sleep only: (esp_light_sleep_start()), supporting multiple GPIOs (including non-RTC pins on many ESP32 variants). You can use either GPIO_INTR_LOW_LEVEL or GPIO_INTR_HIGH_LEVEL. It does not wake the chip from deep sleep.

Practical Takeaway for Your AVR + ESP32 Node
Use esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1) when calling esp_deep_sleep_start()—your single AVR wake line (PIN_WAKE_ESP) must land on an RTC-capable GPIO and assert HIGH (1).

Use gpio_wakeup_enable(GPIO_NUM_33, GPIO_INTR_HIGH_LEVEL) + esp_sleep_enable_gpio_wakeup() when calling esp_light_sleep_start(), especially if you ever want to wake on more than one pin at once or keep pins configured strictly for light sleep.
---------------------------------------------------------------------------
Prompt:
Explain the difference between esp_sleep_enable_ext0_wakeup() and esp_sleep_enable_ext1_wakeup() on ESP32.

Gemini:
Core Difference: Single-Pin Level Vs. Multi-Pin RTC Mask
esp_sleep_enable_ext0_wakeup() uses the RTC IO controller to wake from sleep (deep sleep or light sleep) on one single RTC GPIO when it reaches a specific logic level (0 = LOW, 1 = HIGH).

esp_sleep_enable_ext1_wakeup() uses the RTC controller's external wakeup logic to wake from deep sleep using one or more RTC GPIOs selected via a bitmask (uint64_t mask of 1ULL << gpio_num), triggering when any selected pin goes HIGH (ESP_EXT1_WAKEUP_ANY_HIGH) or when all selected pins go LOW (ESP_EXT1_WAKEUP_ALL_LOW).


