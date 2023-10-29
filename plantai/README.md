# PlantAI

Reads sensors and control actuators for plants to take care of themselves


A single MC board should be able to deal with multiple plants by mux actuators and demux sensors


The data collected is to be sent somewhere via UART or wi-fi.

Sensors and actuators should be part of a physical plant container, easing the plug & play of different containers to the same board


- Nice to have: an interface to create / delete plant containers without re-compiling or restarting


---


##### Sensors

- Soil moisture
https://www.electronicwings.com/arduino
soil-moisture-sensor-interfacing-with-arduino-uno

- Temperature & Humidity
https://www.electronicwings.com/arduino/dht11-sensor-interfacing-with-arduino-uno


- Light level