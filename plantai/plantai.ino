#include <ArduinoJson.h>
#include <DHT.h>

#define DEBUG 1
#define MAX_PLANTS 16

#define SENSOR_INTERVAL_IN_MILLISECONDS 3000
#define SERIAL_BAUDRATE 115200

#define MAX_SOIL_MOISTURE_ANALOG_SIGNAL 1023.0

typedef struct
{
    DHT dht;         // digital
    uint8_t soilPin; // analog
    uint8_t lightPin;
} plantSensor_t;

typedef struct
{
    String id;
    String name;

    float soil_moisture; // %
    float temperature;   // Celsius
    float humidity;      // %
    float light;         // %

    plantSensor_t *sensor;
} plant_t;

plant_t* PLANTS[MAX_PLANTS];
uint8_t numPlants = 0;

// =========================================
// ================  SETUP  ================
// =========================================

void setupPlantSensor(plant_t *plant, uint8_t dhtPin = 53, uint8_t soilPin = A0, uint8_t lightPin = 23)
{
    debug("DHT pin ");
    debugln(dhtPin);
    DHT dht = DHT(dhtPin, DHT11);
    dht.begin();

    debug("soilPin ");
    debugln(soilPin);
    pinMode(soilPin, INPUT);

    debug("lightPin ");
    debugln(lightPin);
    pinMode(lightPin, INPUT);

    plantSensor_t *sensor = (plantSensor_t *)malloc(sizeof(plantSensor_t));
    sensor->dht = dht;
    sensor->soilPin = soilPin;
    sensor->lightPin = lightPin;

    plant->sensor = sensor;
}

void setup()
{
    Serial.begin(SERIAL_BAUDRATE);
    
    if (DEBUG) {

      uint8_t dhtPin = 53;
      uint8_t soilPin = A0;
      uint8_t lightPin = 23;

      String names[] = {"Vincent", "Violenta"};


      for (uint8_t i = 0; i < 2; i++)
      {
          createPlant(String(i), names[i], dhtPin, soilPin + i, lightPin);
      }
    }
 
}

// =========================================
// ================  LOOP  =================
// =========================================

// void loop()
// {
//     plant_t *plant;

//     for (uint8_t i = 0; i < PLANTS_COUNT; i++)
//     {
//         plant = &PLANTS[i];
//         readPlantSensors(plant);
//         serializePlant(plant);
//         delay(SENSOR_INTERVAL_IN_MILLISECONDS);
//         delay(2 * 1000);
//     }
// }

void loop()
{
    if (!Serial.available()) {
      return;
    }

    plant_t* plant;

    String command = Serial.readString();
    debugln(">>> " + command);

    String args = command.substring(1);
    debugln(">>>>" + args);

    switch (command[0]) {
      case '+':
        plant = createPlant("_id", args, 53, A0, 23);
        serializePlant(plant);
        break;
      case '-':
        deletePlant(args);
        break;
      case '=':
        plant = retrievePlant(args);
        serializePlant(plant);
        break;
      case '?':
      default:
        listPlants();
        break;
    }
}

// =========================================
// ==================  API  ================
// =========================================

plant_t* createPlant(const String id, const String name, uint8_t dhtPin, uint8_t soilPin, uint8_t lightPin) {
  debug("Creating Plant ");
  debugln(name);

  if (numPlants >= MAX_PLANTS) {
    debugln("Cant create a plant past maximum plants");
    return NULL;
  }

  plant_t *plant = (plant_t *)malloc(sizeof(plant_t));
  serializePlant(plant);

  plant->id = id;
  plant->name = name;

  debugln(name);
  debugln(plant->name);

  serializePlant(plant);
  setupPlantSensor(plant, dhtPin, soilPin, lightPin);

  PLANTS[numPlants] = plant;
  numPlants++;

  debugln("Plant created.");
  return plant;
}

void deletePlant(String name) {
  debug("Deleting Plant ");
  debugln(name);

  for (uint8_t i = 0; i < numPlants; i++) {
    if (PLANTS[i]->name == name) {
      
      // Shift remaining plants to fill the gap
      for (uint8_t j = i; j < numPlants - 1; j++) {
        PLANTS[j] = PLANTS[j + 1];
      }

      numPlants--;
      debugln("Plant deleted.");
      return;
    }
  }

  debugln("Plant not found.");
}

plant_t* retrievePlant(String name) {
  debug("Retrieving Plant ");
  debugln(name);
  
  for (uint8_t i = 0; i < numPlants; i++) {
    if (PLANTS[i]->name == name) {
      return PLANTS[i];
    }
  }

  debugln("Plant not found");
  return NULL;
}

void listPlants() {
  StaticJsonDocument<256> doc;
  doc["plants"].to<JsonArray>();

  for (uint8_t i = 0; i < numPlants; i++) {
    serializePlant(PLANTS[i]);

    StaticJsonDocument<32> _doc;
    JsonObject plantJson = _doc.to<JsonObject>();
    plantJson["name"] = PLANTS[i]->name;
    
    doc["plants"].add(plantJson);
  }

  serializeJson(doc, Serial);
  Serial.println();
}

void serializePlant(const plant_t *plant)
{
    StaticJsonDocument<256> doc;

    doc["id"] = plant->id;
    doc["name"] = plant->name;
    doc["soil_moisture"] = plant->soil_moisture;
    doc["temperature"] = plant->temperature;
    doc["humidity"] = plant->humidity;
    doc["light"] = plant->light;

    serializeJson(doc, Serial);
    Serial.println();
}
// =========================================
// ===============  SENSORS  ===============
// =========================================

float readSoilMoisture(plantSensor_t *sensor)
{
    int analog_signal = analogRead(sensor->soilPin);
    // float moisture_percentage = (100 - ((sensor_analog / 1023.00) * 100));
    float moisture_percentage = analog_signal / MAX_SOIL_MOISTURE_ANALOG_SIGNAL;

    debug("Read soil moisture signal: ");
    debugln(analog_signal);

    return moisture_percentage;
}

float readTemperature(plantSensor_t *sensor)
{
    float temperature = sensor->dht.readTemperature();

    if (isnan(temperature))
    {
        debugln("Failed to read temperature from DHT sensor!");
        return -1.0;
    }

    debug("Read temperature ");
    debugln(temperature);

    return temperature;
}

float readHumidity(plantSensor_t *sensor)
{
    float humidity = sensor->dht.readHumidity();

    if (isnan(humidity))
    {
        debugln("Failed to read humidity from DHT sensor!");
        return -1.0;
    }

    debug("Read humidity: ");
    debugln(humidity);

    return humidity;
}

float readLight(plantSensor_t *sensor)
{
    return -2.0;
}

void readPlantSensors(plant_t *plant)
{
    plantSensor_t *sensor;

    sensor = plant->sensor;

    plant->soil_moisture = readSoilMoisture(sensor);
    plant->temperature = readTemperature(sensor);
    plant->humidity = readHumidity(sensor);
    plant->light = readLight(sensor);
}


// =========================================
// ================  DEBUG  ================
// =========================================

void debugln(const uint8_t message)
{
    if (DEBUG)
    {
        Serial.println(message);
    }
}

void debug(String message)
{
    if (DEBUG)
    {
        Serial.print(message);
    }
}

void debugln(String message)
{
    if (DEBUG)
    {
        Serial.println(message);
    }
}