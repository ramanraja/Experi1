/*
ESP Now experiment
New in this version: Broadcast to multiple ESP's. Simple string messages instead of structs.
The power consumption stays incredibly low because the radios wakeup only 
  for a fraction of a second to send each packet.
Tip: The MAC address of a board is printed by the compiler output messages.
https://dronebotworkshop.com/esp-now/
https://www.youtube.com/watch?v=bEKjCDDUPaU
https://www.etechnophiles.com/esp32-useful-concepts/

#define ESP_NOW_ETH_ALEN             6    // Length of ESPNOW peer MAC address
#define ESP_NOW_KEY_LEN              16   // Length of ESPNOW peer local master key
typedef struct esp_now_peer_info {
    uint8_t peer_addr[ESP_NOW_ETH_ALEN];  // ESPNOW peer MAC address that is also the MAC address of station or softap
    uint8_t lmk[ESP_NOW_KEY_LEN];         // ESPNOW peer local master key that is used to encrypt data
    uint8_t channel;                      // Wi-Fi channel that peer uses to send/receive ESPNOW data. If the value is 0,
                                          // use the current channel which station or softap is on. Otherwise, it must be
                                          // set as the channel that station or softap is on.
    wifi_interface_t ifidx;               // Wi-Fi interface that peer uses to send/receive ESPNOW data
    bool encrypt;                         // ESPNOW data that this peer sends/receives is encrypted or not
    void *priv;                           // ESPNOW peer private data
} esp_now_peer_info_t;
NOTEs:
- Right click the struct name on Arduino IDE and select 'go to definition' to open the header file containing the struct.
  (Tip: for this to work, you need to select the board type to be ESP32 on the IDE)
- esp_now_peer_info_t peerInfo = {}; zero-initializes everything, including lmk, ifidx, and priv. This is good practice 
because esp_now_add_peer() reads all the fields, and uninitialized garbage in ifidx or priv is a common cause of intermittent failures.
- channel = 0 means “use whatever channel the Wi-Fi interface is currently on.” If you set a nonzero value, it must match the channel 
your station or softAP is actually on.
- encrypt = false means lmk is ignored. If you set it to true, you’d also need to fill lmk with a 16-byte key.
- ifidx defaults to 0 after zero-initialization, which is WIFI_IF_STA. If you’re using the softAP interface, 
set peerInfo.ifidx = WIFI_IF_AP;.
*/
#ifdef ARDUINO_ARCH_ESP8266
  #include <ESP8266WiFi.h>
  #include <espnow.h>
#elif defined(ARDUINO_ARCH_ESP32)
  #include <WiFi.h>
  #include <esp_now.h>
#else
  #error "Only ESP32/ESP8266 boards are supported"
#endif

// Keep one of these on each device and comment out the others
//#define SELF_ID     "AAA"  // 30:ae:a4:03:2f:38
#define SELF_ID   "BBBB"     // 40:91:51:fc:c6:44
//#define SELF_ID   "CCCCC"  // 40:f5:20:26:04:94

//uint8_t broadcastAddress[] = {0x30,0xAE,0xA4,0x03,0x2F,0x38}; // hard coded MAC of our Rx 
uint8_t broadcastAddress[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};   // this makes the difference: it is broadcast mode
// change this, if you want to target a particular board

char tx_buffer[ESP_NOW_MAX_DATA_LEN + 1];   // outgoing message
char rx_buffer[ESP_NOW_MAX_DATA_LEN + 1];   // incoming message
char src_mac_address[20];  // source MAC address of the incoming message

// This is the new callback signature for ESP32 core 3.x
void OnDataSent(const wifi_tx_info_t *mac_info, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "....Delivered." : "....Delivery Failed.");
}

// NEW callback signature for ESP32 core 3.x / IDF v5
void onDataReceived(const esp_now_recv_info_t *recv_info,
                   const uint8_t *incomingData,
                   int len) {
  int msgLen = min(ESP_NOW_MAX_DATA_LEN, len);
  strlcpy(rx_buffer, (const char *)incomingData, msgLen);
  format_mac_address(recv_info->src_addr); // this is stored in the global src_mac_address
  Serial.print("<< ");
  Serial.print(len);
  Serial.print(" bytes from ");
  Serial.print(src_mac_address);
  Serial.print(" -> ");
  Serial.println(rx_buffer);
}

// helper function to store the incoming MAC address into a global scratch pad
void format_mac_address(const uint8_t *macAddr)
{
  snprintf(src_mac_address, 18, "%02x:%02x:%02x:%02x:%02x:%02x", 
  macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

void setup() 
{
  Serial.begin(115200);
  while (!Serial);  
  Serial.println("\n\nESP Now broadcaster starts...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-Now init failed");
    return;
  }
  Serial.println("ESP-Now initialized successfully");
  Serial.print("My node ID: ");
  Serial.println(SELF_ID);
  
  // Register 2 callbacks - for send and receive  
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(onDataReceived);
 
  // Note how the struct esp_now_peer_info_t is initialized:
  // struct s = {} initializes all fields to zero
  esp_now_peer_info_t peerInfo = {};
  // peerInfo.peer_addr and broadcastAddress are both arrays, so you can memcpy() 
  // between them without using the address operator '&'
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0; // use whatever channel your WiFi interface is already on
  peerInfo.encrypt = false;
  // Add receiver peer, if not already existing  
  if (esp_now_is_peer_exist(broadcastAddress)) {
    Serial.println("ESP Now peer already exists!");
  } else {
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      Serial.println("Failed to add ESP-Now peer");
      return;
    }
    Serial.println("ESP-Now peer added.");    
  }
}

uint16_t count = 0; 
void loop()
{
  snprintf(tx_buffer, 48, "<BOF>This is message# %d from %s<EOF>", count, SELF_ID);
  Serial.print("   >> broadcasting packet#: ");
  Serial.print(count++);
  // You MUST send the null terminator: strlen() does not count it!
  esp_err_t result = esp_now_send (broadcastAddress,
                                  (uint8_t*)tx_buffer,
                                  strlen(tx_buffer)+1);
  if (result == ESP_OK) 
    Serial.print(" -> OK");
  else 
    Serial.print(" -> Error");
  delay(random(4000, 5000));  // jitter
}
