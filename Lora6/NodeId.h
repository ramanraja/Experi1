#ifndef NODEID_H
#define NODEID_H

// ESP32 and 8266 require ISRs to be placed in IRAM; AVR does not use this attribute:
// The LoRa.onReceive() callback should not access external Flash as it is executing inside an ISR.
// NOTE: if you call a helper method like MyQueue.push() inside the callback, that also has to be
// marked as IRAM_ATTR; this flag keeps the method entirely in RAM, without having to fetch it from Flash.
#if defined(ESP8266) || defined(ESP32)
  #define ISR_PREFIX IRAM_ATTR
#else
  #define ISR_PREFIX
#endif

#define  BROADCAST_ID      0xFF

// Every LoRa node should have a unique id in the range 0x00 to 0xFF
// Retain any one of the following and comment out all others

// Master node id
#define  MASTER_ID          0x55  
#define  NODE_ID            0x55    // enable this for the master
  
// Slave node ids: enable one
//#define  NODE_ID            0xAA 
//#define  NODE_ID              0xBB  
//#define  NODE_ID            0xCC  


#endif