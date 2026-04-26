# PlantAI

Reads sensors and control actuators for plants to take care of themselves


A single MC board should be able to deal with multiple plants by mux actuators and demux sensors


The data collected is to be sent somewhere via UART or wi-fi.

Sensors and actuators should be part of a physical plant container, easing the plug & play of different containers to the same board


### Commands

1. List - Fetch every plant currently know by Arduino
```
>>> ?
<<< {
    "plants": [
        {
            // plant1
        },
        {
            // plant2
        },
    ]
}
```

2. Retrieve - Look for a plant by name and read its sensors before responding
```
>>> =PLANT_NAME
<<< {
    // plant with name == PLANT_NAME
}
```

3. Create - Let Arduino know about a plant with its sensors

```
>>> +{"name":"PLANT_NAME","pins":{"soil":2,"dht":3,"light":4}}
<<< {
    // created plant
}
```

4. Delete - Let Arduino forget about a plant by it's name
```
>>> -PLANT_NAME
<<< {
    // deleted plant
}
```

---

### References

##### Sensors 

[Soil moisture](https://www.electronicwings.com/arduino/soil-moisture-sensor-interfacing-with-arduino-uno)

[Temperature & Humidity](https://www.electronicwings.com/arduino/dht11-sensor-interfacing-with-arduino-uno)

[Light level](https://www.instructables.com/Arduino-and-a-LDR-Light-Dependent-Resistor/)

---
