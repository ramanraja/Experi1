// common.h
#ifndef COMMON_H
#define COMMON_H
#include <Arduino.h>

#define APP_NAME          "LoRa App"
#define FIRMWARE_VERSION  "1.1"

#define ENABLE_DEBUG

// Some stupid libraries have defined macros SP and SPL !
#ifdef ENABLE_DEBUG
  #undef   SP
  #undef   SPL
  #define  SW(x)      Serial.write(x)
  #define  SWN(x, n)  Serial.write(x, n)
  #define  SP(x)      Serial.print(x)
  #define  SPL(x)     Serial.println(x)
  #define  SPF(...)   Serial.printf(__VA_ARGS__)    
#else
  #define  SW(x)  
  #define  SWN(x, n) 
  #define  SP(x)     
  #define  SPL(x)   
  #define  SPF(...)  
#endif

 
#endif
