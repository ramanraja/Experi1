// mm Wave radar
/*
Waveshare Human Micro-Motion Detection mmWave Sensor, 24GHz mmWave Radar: 
Based On S3KM1110, FMCW Technology.
Wiring:
Power the radar from the 3.3V output of the NodeMCU.
Leave the Tx and Rx pin unconnected.
Connect the digital out pin (OT2) to GPIO 5 of NodeMCU.

Config tool:
Download the mmwave Radar configuration tool from the link on the wiki page:
https://www.waveshare.com/wiki/HMMD_mmWave_Sensor
The link to the zip file is:
https://files.waveshare.com/wiki/HMMD-mmWave-Sensor/HMMD-mmWave-Sensor-Tools.zip

Connect the radar to your PC through a 3.3 V USB to serial cable  
Run the exe file and set the number of gates to 3 for a typical bath room use.
*/
#define RADAR    5
#define LED1     2   // Node MCU
#define LED2     16

void setup() {
  //Serial.begin(115200);
  pinMode(RADAR, INPUT);
  pinMode(LED1, OUTPUT);  // active low
  pinMode(LED2, OUTPUT);  // active low
  blink();
}

void loop() {
  digitalWrite(LED2, !digitalRead(RADAR));
  delay(20);
}

void blink() {
  for(int i=0; i<4; i++) {
    digitalWrite(LED1, 0);
    delay(100);
    digitalWrite(LED1, 1);
    delay(100);    
  }
}