# Q-Sys-BME280-Temp-Humidity
This is a Q-Sys plugin and associated Arduino code to create a low cost open source environmental sensor peripheral for the Q-Sys architecture.

## Overview
The BME280 Temp & Humidity is a network environmental sensor peripheral designed for the Q-SYS architecture. The peripheral utilizes the commonly available Bosch BME280 environmental sensor supplying Q-Sys with realtime temperature, barometric pressure and humidity readings. Built around readily available open-source hardware the Remote GPIO Nano can be assembled for around $25 per unit. An Arduino Nano microcontroller in companion with an w5500 ethernet shield enables UDP communication with the Q-SYS core. This plugin allows for ease of setup and integration into an existing design appearing as any other configurable GPIO component. 

## Properties
- Name
  - *Used to configure a custom name for the plugins status*

## Control Pin Definitions
- Analog Reference Voltage (Voltage use to refence the analog pins against, also the Arduinos input voltage. *Should not exceed 5.5v)
  - ***Read Only*** [0.00v - 5.50v]
- Device IP (IP Address of The Remote GPIO Nano)
  - ***Read, Write***
- Device Port (UDP port the The Remote GPIO Nano is listening on)
  - ***Read, Write***
- Firmware (The firmware version running on the Arduino.)
  - ***Read Only*** [text string formatted TH#-#]
- Humidity (The humidity data from the BME280. *±3% accuracy)
  - ***Read Only*** [0% - 100%]
- Pressure (The barometric pressure data from the BME280. *±1 hPa absolute accuraccy)
  - ***Read Only*** [300Pa - 1100 hPa]
- Status (The status of the plugin component)
  - ***Read Only*** [text string]
- Temperature (The temperature data from the BME280. *±1.0°C accuracy)
  - ***Read Only*** [-40°C - 85°C]
- Units (Specify either Metric (SI) or US (USCS) units)
  - ***Read, Write***
- Up Time (Up time of the the BME280 Temp & Humidity)
  - ***Read Only***


## HARDWARE

- Arduino Nano
  
- w5500 Arduino Ethernet Shield
  
- BME280 Sensor

## SETUP

*This assumes that you are familiar with how to program an Arduino and install corresponding libraries.*

After installing the Arduino IDE and connecting the Arduino Nano via USB to the PC you can start by opening the BME280-Temp-Humidity-Config file. The Config file allows you to modify the default static IP address, choose between a Static or DHCP IP, select the UDP communication port, and set the device Hostname. Editing of the Config file is optional as all these settings can later be changed through the web UI accessible though Q-Sys configurator. Once ready the Config file can then be uploaded to the Arduino, this Config file will only need to be uploaded once onto a new Arduino board as it creates the necessary framework for the default settings in the eeprom of the Arduino. After the Config file has been successfully uploaded to the board you can then open the Q-SYS-BME280-Temp-HumidityV1 file in the Arduino IDE, simply upload this file to the Arduino board. Congratulations you have now completed the most difficult part of the setup. Now for the easy part of setting up the hardware. Depending on where you sourced the hardware from you may need to solder the header pins onto the Arduino and ethernet shield, typically they come presoldered making setup easy. With the pins attached the Arduino Nano will plug into the ethernet shield and pins A4 and A5 will connect to the SDA and SCL pins of the BME280 sensor. Lastly plug in the ethernet cable and connect 5v power to the board via a micro-USB cable and USB power supply or a 5v power supply connected to the Vin and GND pins of the Arduino. This completes the Arduino portion of the setup, and the hardware will be ready to go.

  After installing the Q-Sys plugin drag the component into the design and configure it as any other GPIO component. Once the design is running the IP Address and UDP port can be entered in the Setup/Status tab. To find the IP Address and UDP port after the hardware has been setup and plugged in navigate to Q-Sys Configurator and look under I/O Devices > I/O-Frame8S and you should see a device named "ArduinoNano" click on the device and select the "Open Peripheral Manager" link in the right-side pane. *If there is an error where Configurator shows the IP Address as 0.0.0.0 press the reset button on the Arduino and it should fix the problem. This should open a web browser to the IP Address of the Remote GPIO Nano, this is where you can now change the settings from before in the simple web UI. Once the settings have been changed press submit and the Remote GPIO Nano will reset and temporarily drop off the network as the new settings take effect.
