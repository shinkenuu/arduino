#include <ArduinoJson.h>
#include <DHT.h>

#define DEBUG false

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
    char id[10];
    char name[30];

    float soil_moisture; // %
    float temperature;   // Celsius
    float humidity;      // %
    float light;         // %

    plantSensor_t *sensor;
} plant_t;

plant_t plant0 = {"bb3ca75", "Violeta", -1.0, -1.0, -1.0, -1.0, NULL};
plant_t plant1 = {"2a75cb8", "Bruminha", -1.0, -1.0, -1.0, -1.0, NULL};
plant_t plant2 = {"c54122a", "Christine", -1.0, -1.0, -1.0, -1.0, NULL};

plant_t PLANTS[] = {plant0, plant1, plant2};
uint8_t PLANTS_COUNT = sizeof(PLANTS) / sizeof(plant_t);

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

    plant_t *plant;
    uint8_t dhtPin = 53;
    uint8_t soilPin = A0;
    uint8_t lightPin = 23;

    for (uint8_t i = 0; i < PLANTS_COUNT; i++)
    {
        debug("Setting up plant ");
        debugln(i);

        plant = &PLANTS[i];
        setupPlantSensor(plant, dhtPin, soilPin + 1, lightPin);

        debugln("Setup plant");
    }
}

// =========================================
// ================  LOOP  =================
// =========================================

void loop()
{
    plant_t *plant;

    for (uint8_t i = 0; i < PLANTS_COUNT; i++)
    {
        plant = &PLANTS[i];
        readPlantSensors(plant);
        serializePlant(plant);
        delay(SENSOR_INTERVAL_IN_MILLISECONDS);
    }
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
// =================  COMM  ================
// =========================================

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

void debugln(const uint8_t message)
{
    if (DEBUG)
    {
        Serial.println(message);
    }
}

void debug(const char *message)
{
    if (DEBUG)
    {
        Serial.print(message);
    }
}

void debugln(const char *message)
{
    if (DEBUG)
    {
        Serial.println(message);
    }
}