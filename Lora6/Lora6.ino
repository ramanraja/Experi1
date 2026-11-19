/*
LORA Transreceiver: This is the central Hub/ Master node; it receives data from sensor nodes (slaves)
New in this version: (1) Wrapping Structure to capture RSSI and SNR of each packet 
https://robu.in/product/sx1278-lora-module-ra-02-433mhz-wireless-spread-spectrum-transmission/
https://robu-prod-media.s3.ap-south-1.amazonaws.com/products/attachments/m6SzfPnJeaLg1zNQpvstWaTr9IkJnCwabJtIAq1U.docx
https://github.com/sandeepmistry/arduino-LoRa
Connections:
SX1278 	ESP32  
VCC		  3.3V
GND		  GND
MISO		GPIO 19
MOSI		GPIO 23
SCK		  GPIO 18
NSS/CS	GPIO 5
RESET		GPIO 14
DIO0		GPIO 26  (in some examples, GPIO 2 or 4 are also suggested for DIO0)
*/
 
#include <SPI.h>
#include <LoRa.h>   // https://github.com/sandeepmistry/arduino-LoRa
#include <Timer.h>  // https://github.com/JChristensen/Timer/  
#include "common.h"
#include "NodeId.h"

#define  TICK_INTERVAL        5000
#define  TEMP_BUFFER_SIZE     64
#define  QUEUE_SIZE           8      // FIFI queue of DataPacketWrapper objects
#define  ALPHA                0.1    // for Exponential Moving Averages
#define  ONE_MINUS_ALPHA      0.9
// LoRa configuration
#define  DATA_SIZE            16     // this accounts only for the string 'payload' inside the structure
#define  LORA_FREQUENCY       433E6  // or 915E6, or 868E6, depending on the MHz band of your LoRa module
#define  TX_POWER             7      // recommended <= 10; max 20 (caution!)
#define  SPREAD_FACTOR        7      // must match the receiver
#define  NETWORK_SYNC_WORD    0xA5   // distinguishes our network from the neighbours' networks
// Hardware Pins
#define  CS                   5      // connected to the chip select pin of the LoRa module
#define  RESET                14     // connected to the reset pin of the LoRa module
#define  DIO0                 26     // interrupt/wake up the CPU; GPIO 2 and GPIO 4 are also usable

typedef struct {
  uint16_t pid;   // packet serial number
  uint8_t  from;  // sender's own device id
  uint8_t  to;    // targeted recipient's id; 0xFF to broadcast
  char payload[DATA_SIZE];  // sensor data
} DataPacket;

typedef struct {
  DataPacket data;
  int16_t  RSSI;
  float    SNR;
} DataPacketWrapper;

DataPacket  tx_data; // temporary buffer to hold outbound data
DataPacketWrapper  queue[QUEUE_SIZE];
float chl_rssi = 0.0;  // TODO: make these per sensor node on the Master 
float chl_snr = 0.0;

// Note: head and tail are just array indices, not pointers into the queue
volatile uint8_t head = 0;  // main loop empties queue at the head
volatile uint8_t tail = 0;  // the callback ISR writes new arrivals at the tail

uint packet_count = 0;  // total received packets, including defective and dropped packets
uint malformed_packet_count = 0;   // packets not conforming to the 'DataPacket' structure
uint queue_overflow_count = 0;     // queue was full when fresh data arrived; packet got dropped
char temp_buffer[TEMP_BUFFER_SIZE];   // scratchpad display formatter
uint8_t  sensor_nodes[] = {0xAA, 0xBB, 0xCC};
bool lora_ok = false; 
Timer T;

// Caution: This function is called in the DIO0 interrupt context
// ISR_PREFIX is defined in NodeId.h
// NOTE: do NOT perform anything like Serial.print() that uses interrupts inside an ISR! 
void ISR_PREFIX receive_callback(int packetSize){
    if (packetSize != sizeof(DataPacket)) {   // wrong payload
      while (LoRa.available()) LoRa.read();   //  drain and ignore the packet
      malformed_packet_count++;
      return;
    }
    uint8_t next_slot = (tail + 1) % QUEUE_SIZE;
    if (next_slot == head) {    // queue ful
      while (LoRa.available()) LoRa.read();   // drain and ignore the packet
      queue_overflow_count++;    
        return;                     
    }
    // put the received bytes into the inner DataPacket structure within the wrapper object
    LoRa.readBytes((uint8_t*)&queue[tail].data, sizeof(DataPacket)); 
    queue[tail].RSSI = LoRa.packetRssi();
    queue[tail].SNR = LoRa.packetSnr();
    // Compiler memory barrier
    asm volatile("" ::: "memory");
    // finally, publish the new tail
    tail = next_slot;
}

void setup() 
{
    Serial.begin(115200);
    while (!Serial);  
    SPF("\n%s Ver %s starts...\n", APP_NAME, FIRMWARE_VERSION);
    if (init_lora()) 
        T.every(TICK_INTERVAL, tick);
}

bool init_lora() 
{    
    // select the pins to which the subsequent commands will be sent: 
    //This MUST be called BEFORE LoRa.begin()
    LoRa.setPins(CS, RESET, DIO0); // the library default pins are configured for AVR boards
    // reset the LoRa module and let it stabilize to its default configuration 
    // We are using a 433 MHz LoRa module SX 1278
    if (!LoRa.begin(LORA_FREQUENCY))    
    {                                                                    
      SPL(F("\nLoRa initialization failed! Hibernating......."));
      return false;
    }
    // Send radio configurations to the module AFTER LoRa.begin()
    // Set transmission power to a safe 7 dBm, so that 3.3V pin of ESP32 can drive it
    // Higher power output needs separate 3.3V power supply to the LoRa module
    // Allowed range is 2 dBm to 20 dBm; library default is 17 dBm
    // Legal limit is 10 dBm; 7 dBm is a sweet spot to start with
    LoRa.setTxPower(TX_POWER);    
    //Range tuning: you can trade range for speed with LoRa.setSpreadingFactor(7 to 12) 
    //(higher = longer range, but slower/more power) and LoRa.setSignalBandwidth(). 
    //Default settings (SF7 and 125kHz) are a reasonable starting point.
    // Optional: adjust spreading factor - this MUST match the transmitter
    LoRa.setSpreadingFactor(SPREAD_FACTOR);    
    // Optional: Adjust sync word - network identifier. (0xF3 is standard, or use 0x12 for private networks)
    ///LoRa.setSyncWord(NETWORK_SYNC_WORD);  // this MUST match the receiver's network filter 
    //  enable checksum (should do it on both send and receive sides):
    LoRa.enableCrc();
    SPL(F("LoRa Initialized Successfully!\n"));  
    LoRa.onReceive(receive_callback); // register callback - uses DIO0 interrupt
    SPL(F("Receive callback registered."));
    SPF(" Node id: %2X\n", NODE_ID);  // from NodeId.h
    SPF(" Frequency band: %ld\n", LORA_FREQUENCY);
    ///SPF(" Sync word: 0x%2X\n", NETWORK_SYNC_WORD);  // if you are changing the default
    SPF(" Transmit power: %d dBm\n", TX_POWER);
    SP(F(" Spreading factor: "));  SPL(SPREAD_FACTOR);
    lora_ok = true;
    LoRa.receive();  // Important: go into continuous receive mode (not send-and-sleep mode)
    return lora_ok;
}
 
void loop()
{   
    if (!lora_ok) return;
    T.update();
    if (head != tail)   // queue is not empty
      parse_payload();
}

// binary data
void parse_payload (){
    if (queue[head].data.to == NODE_ID || queue[head].data.to == BROADCAST_ID) {
      packet_count++; // total number of received packets meant for this node
      snprintf(temp_buffer, TEMP_BUFFER_SIZE-1, "<< %d| %d| 0x%2X| 0x%2X| %d| %.2f| %s", 
        head,
        queue[head].data.pid,
        queue[head].data.from,
        queue[head].data.to,
        queue[head].RSSI, 
        queue[head].SNR,   
        queue[head].data.payload 
      );
      compute_ema(queue[head].RSSI, queue[head].SNR); // exponential moving average
    } else {  // not addressed to me
      snprintf(temp_buffer, TEMP_BUFFER_SIZE-1, "< %d| %d| 0x%2X| 0x%2X (ignored)", 
        head,
        queue[head].data.pid,
        queue[head].data.from,
        queue[head].data.to
      );    
    }
    SPL(temp_buffer);
    head = (head+1) % QUEUE_SIZE;
  
}

void tick() {
    static int counter = 0;
    tx_data.pid = counter;
    tx_data.from = NODE_ID;  // from NodeId.h
    if (NODE_ID == MASTER_ID) {
      tx_data.to = sensor_nodes[counter % 3]; // there are 3 slave nodes in our network
      snprintf(tx_data.payload, DATA_SIZE-1, "Master# %d", counter);
    } else {
      tx_data.to = MASTER_ID;   // a slave sends only to the master node
      snprintf(tx_data.payload, DATA_SIZE-1, "Sensor# %d", counter);      
    }
    SP(F("  > "));
    SPL(tx_data.payload);
    counter++;
    LoRa.beginPacket();  // binary payload
    LoRa.write((uint8_t*)&tx_data, sizeof(tx_data));  // use LoRa.print() for text paload
    LoRa.endPacket();  // after sending the module will go back to sleep mode. so...
    // ...put it back in receive mode:
    LoRa.receive();   // only needed if this node also listens
    // TODO: experiment how to send it in non-blocking mode:
    // LoRa.endPacket(true); // true = async / non-blocking mode
    if (counter % 9 == 0) report();
}

void report(){
  SPL(F("-----------------------------------"));
  SP(F("Total packets received: ")); SPL(packet_count);
  SP(F("Malformd packets: ")); SPL(malformed_packet_count);
  SP(F("Dropped packets (Q full): ")); SPL(queue_overflow_count);
  SPF("Average RSSI: %.1f dBm\n", chl_rssi);
  SPF("Average SNR %.2f dB\n", chl_snr);
  SPL(F("-----------------------------------"));
}

// Exponential Moving Average
// average = ALPHA * new_value + (1-ALPHA) * previous_average
// ALPHA = 0.05 -> slow convergence, but smoother; 
// ALPHA = 0.5  -> fast convergence
// ALPHA = 0.1 or 0.2 are recommended
// If we start from 0.0 initial value, it will slowly ramp up to the average, so we 
// seed it with the initial reading
void compute_ema(int rssi, float snr) {
  static bool first_sample = true;
  if (first_sample) {
    first_sample = false;
    chl_rssi = rssi;
    chl_snr = snr;
  } else {
    chl_rssi = ALPHA * rssi + ONE_MINUS_ALPHA * chl_rssi;
    chl_snr = ALPHA * snr + ONE_MINUS_ALPHA * chl_snr;
  }
}
 