#include <ArduinoJson.h>
#include <DHT.h>

#define DEBUG 0
#define MAX_PLANTS 16

#define SENSOR_INTERVAL_IN_MILLISECONDS 3000
#define SERIAL_BAUDRATE 115200

#define MAX_SOIL_MOISTURE_ANALOG_SIGNAL 1023.0


// =========================================
// ================  PLANT  ================
// =========================================

typedef struct
{
  DHT dht;          // digital
  uint8_t soilPin;  // analog
  uint8_t lightPin;
} plantSensor_t;

typedef struct
{
  String id;
  String name;

  float soil_moisture;  // %
  float temperature;    // Celsius
  float humidity;       // %
  float light;          // %

  plantSensor_t *sensor;
} plant_t;

plant_t PLANTS[MAX_PLANTS];
uint8_t numPlants = 0;

void setupPlantSensor(plant_t *plant, uint8_t dhtPin = 53, uint8_t soilPin = A0, uint8_t lightPin = 23) {
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


// =========================================
// ==============  ARDUINO  ================
// =========================================

void setup() {
  Serial.begin(SERIAL_BAUDRATE);

  if (DEBUG) {

    uint8_t dhtPin = 53;
    uint8_t soilPin = A0;
    uint8_t lightPin = 23;

    String names[] = { "Vincent", "Violenta" };


    for (uint8_t i = 0; i < 2; i++) {
      createPlant(String(i), String(names[i]), dhtPin, soilPin + i, lightPin);
    }
  }
}


void loop() {
  if (!Serial.available()) {
    return;
  }

  plant_t *plant = NULL;
  String plantId;

  String command = Serial.readString();
  command.trim();

  char operation = command[0];
  String args = command.substring(1);

  debugln("operation: |" + String(operation) + "|");
  debugln("args: |" + args + "|");

  switch (operation) {
    case '+':
      plant = createPlant("_id", args, 53, A0, 23);
      if (plant == NULL) {
        serializeError("plant not created");
        break;
      }
      readPlantSensors(plant);
      serializePlant(plant);
      break;
    case '-':
      plantId = deletePlant(args);
      if (plantId == "") {
        serializeError("plant named" + args + " not deleted");
        break;
      }
      Serial.println("{\"id\": \"" + plantId + "\"}");
      break;
    case '=':
      debugln("Retriving");
      plant = retrievePlant(args);
      if (plant == NULL) {
        serializeError("plant named " + args + " not found");
        break;
      }
      readPlantSensors(plant);
      serializePlant(plant);
      break;
    case '?':
      debugln("Listing");
    default:
      serializePlants();
      break;
  }
}


// =========================================
// ==============    API    ================
// =========================================

plant_t *createPlant(const String id, const String name, uint8_t dhtPin, uint8_t soilPin, uint8_t lightPin) {
  debugln("Creating Plant " + name);

  if (numPlants >= MAX_PLANTS) {
    debugln("Cant create a plant past maximum plants");
    return NULL;
  }

  plant_t plant;
  plant.id = id;
  plant.name = name;
  setupPlantSensor(&plant, dhtPin, soilPin, lightPin);

  PLANTS[numPlants] = plant;
  numPlants++;

  debugln("Plant created @ " + String(numPlants - 1));
  return &PLANTS[numPlants - 1];
}

String deletePlant(String name) {
  debugln("Deleting Plant " + name);

  String deletedPlantId = "";

  for (uint8_t i = 0; i < numPlants; i++) {
    if (PLANTS[i].name != name) continue;

    deletedPlantId = PLANTS[i].id;

    // Shift remaining plants to fill the gap
    for (uint8_t j = i; j < numPlants - 1; j++) {
      PLANTS[j] = PLANTS[j + 1];
    }

    numPlants--;

    debugln("Plant deleted @ " + String(i));
    return deletedPlantId;
  }

  debugln("Plant not found.");
  return "";
}

plant_t *retrievePlant(String name) {
  debugln("Retrieving Plant " + name);

  for (uint8_t i = 0; i < numPlants; i++) {
    debugln("Comparing name with " + PLANTS[i].name);

    if (PLANTS[i].name.equalsIgnoreCase(name)) {
      debugln("Found plant @ " + String(i));
      return &PLANTS[i];
    }
  }

  debugln("Plant not found");
  return NULL;
}


// =========================================
// ================  COMM  =================
// =========================================

void serializePlants() {
  debugln("Serializing Plants");

  StaticJsonDocument<64 * MAX_PLANTS> doc;
  doc["plants"].to<JsonArray>();

  for (uint8_t i = 0; i < numPlants; i++) {

    StaticJsonDocument<64> _doc;
    JsonObject plantJson = _doc.to<JsonObject>();

    plantJson["id"] = PLANTS[i].id;
    plantJson["name"] = PLANTS[i].name;
    plantJson["soil_moisture"] = PLANTS[i].soil_moisture;
    plantJson["temperature"] = PLANTS[i].temperature;
    plantJson["humidity"] = PLANTS[i].humidity;
    plantJson["light"] = PLANTS[i].light;

    doc["plants"].add(plantJson);
  }

  serializeJson(doc, Serial);
  Serial.println();
}

void serializePlant(const plant_t *plant) {
  debugln("Serializing Plant");

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

void serializeError(String errorMessage) {
  StaticJsonDocument<256> doc;

  doc["error"] = errorMessage;

  serializeJson(doc, Serial);
  Serial.println();
}


// =========================================
// ===============  SENSORS  ===============
// =========================================

float readSoilMoisture(plantSensor_t *sensor) {
  int analog_signal = analogRead(sensor->soilPin);
  // float moisture_percentage = (100 - ((sensor_analog / 1023.00) * 100));
  float moisture_percentage = analog_signal / MAX_SOIL_MOISTURE_ANALOG_SIGNAL;

  debug("Read soil moisture signal: ");
  debugln(analog_signal);

  return moisture_percentage;
}

float readTemperature(plantSensor_t *sensor) {
  float temperature = sensor->dht.readTemperature();

  if (isnan(temperature)) {
    debugln("Failed to read temperature from DHT sensor!");
    return -1.0;
  }

  debug("Read temperature ");
  debugln(temperature);

  return temperature;
}

float readHumidity(plantSensor_t *sensor) {
  float humidity = sensor->dht.readHumidity();

  if (isnan(humidity)) {
    debugln("Failed to read humidity from DHT sensor!");
    return -1.0;
  }

  debug("Read humidity: ");
  debugln(humidity);

  return humidity;
}

float readLight(plantSensor_t *sensor) {
  return -2.0;
}

void readPlantSensors(plant_t *plant) {
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

void debugln(const uint8_t message) {
  if (DEBUG) {
    Serial.println(message);
  }
}

void debug(String message) {
  if (DEBUG) {
    Serial.print(message);
  }
}

void debugln(String message) {
  if (DEBUG) {
    Serial.println(message);
  }
}