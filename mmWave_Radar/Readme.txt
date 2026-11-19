Waveshare MM Wave Radar sensor (HMMD sensor): 
Human Micro-Motion Detection mmWave Sensor, 24GHz mmWave Radar, Based On S3KM1110, FMCW Technology
Hi-Link HLK-LD2420 24GHz Human Motion Radar Sensor

(You may have to add a capacitor to stabilize readings)

https://github.com/2Grey/s3km1110
https://www.waveshare.com/wiki/HMMD_mmWave_Sensor
Test and configuration software:
https://files.waveshare.com/wiki/HMMD-mmWave-Sensor/HMMD-mmWave-Sensor-Tools.zip

NOTE: The mm Wave radar works at 3.3 Volts. You can power it from the 3.3 V pin of a Node MCU.

CAUTION: My FTDI adapter supplied 5 Volts. So connected the 3.3V power supply from a Node MCU to the radar and
connected the NodeMCU ground pin to the serial adapter ground.
NOTE: The FTDI connection has been shown wrongly in the Waveshare Wiki page! The right connection is to
cross connect Rx and Tx to the radar:
  TX ---> RX
  RX ---> TX

Download the mmwave Radar configuration tool from the link on the wiki page:
https://www.waveshare.com/wiki/HMMD_mmWave_Sensor
The link to the zip file is:
https://files.waveshare.com/wiki/HMMD-mmWave-Sensor/HMMD-mmWave-Sensor-Tools.zip

Connect the sensor to your PC through a USB to serial adapter at 3.3V and run the tool 
ICLM_XenD101MM01_HP02Tool.exe

For my bathroom use, I set the Range Gate parameter as 3. This roughly 3 x 0.7 = 2.1 meters
After changing the gate parameter, press the button 'Set Sensor Config' for it to take effect.
This is a one time setup.
 
Wiring to Node MCU:
Power the radar from the 3.3V output of the NodeMCU.
Leave the Tx and Rx pin unconnected.
Connect the digital out pin (OT2) to GPIO 5 of NodeMCU.


Links:
https://robu.in/product/waveshare-human-micro-motion-detection-mmwave-sensor-24ghz-mmwave-radar-based-on-s3km1110-adopts-fmcw-technology/
https://zbotic.in/product/hi-link-hlk-ld2420-24ghz-human-body-micro-motion-sensing-detection/
https://robu.in/product/hlk-ld2420-24ghz-human-body-micro-motion-sensing-detection-radar-sensor-module-hlk-ld2410-low-cost-solution/

Manual:
https://files.seeedstudio.com/wiki/mmWave-radar/MR24HPC1_User_Manual-V1.5.pdf
https://files.seeedstudio.com/wiki/mmWave-radar/24GHz_mmWave_Sensor-Human_Static_Presence_Module_Lite_Datasheet.pdf

Wiki:
https://www.waveshare.com/wiki/HMMD_mmWave_Sensor
https://homey.app/en-us/wiki/mmwave-vs-pir-presence-and-motion-sensors-explained/
https://forum.seeedstudio.com/t/mmwave-human-detection-kit-interference-detection-problems/274677
https://forum.seeedstudio.com/t/mmwave-human-detection-kit-interference-detection-problems/274677/32

Youtube:
https://www.youtube.com/watch?v=I508XQrjvDY
https://www.youtube.com/watch?v=nMVAo3xJZtQ

Github:
https://github.com/2Grey/s3km1110