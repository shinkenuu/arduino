#include <DHT.h>

#define DEBUG 0

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

plant_t PLANTS[] = {plant0, plant1};
uint8_t PLANTS_COUNT = sizeof(PLANTS) / sizeof(plant_t);

// =========================================
// ================  SETUP  ================
// =========================================

void setupPlantSensor(plant_t *plant, uint8_t dhtPin = 53, uint8_t soilPin = A0, uint8_t lightPin = 23)
{
    Serial.print("DHT pin ");
    Serial.println(dhtPin);
    DHT dht = DHT(dhtPin, DHT11);
    dht.begin();

    Serial.print("soilPin ");
    Serial.println(soilPin);
    pinMode(soilPin, INPUT);

    Serial.print("lightPin ");
    Serial.println(lightPin);
    pinMode(lightPin, INPUT);

    plantSensor_t *sensor = (plantSensor_t *)malloc(sizeof(plantSensor_t));
    sensor->dht = dht;
    sensor->soilPin = soilPin;
    sensor->lightPin = lightPin;

    plant->sensor = sensor;
}

void setup()
{
    Serial.begin(115200);

    plant_t *plant;
    uint8_t dhtPin = 53;
    uint8_t soilPin = A0;
    uint8_t lightPin = 23;

    for (uint8_t i = 0; i < PLANTS_COUNT; i++)
    {
        if (DEBUG)
        {
            Serial.println("============================");
            Serial.print("Setting up plant ");
            Serial.println(i);
        }

        plant = &PLANTS[i];
        setupPlantSensor(plant, dhtPin, soilPin + 1, lightPin);

        if (DEBUG)
        {
            Serial.print("Set up plant ");
            Serial.println(i);
        }
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
        delay(3000);
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

    if (DEBUG)
    {
        Serial.print("Read soil moisture signal: ");
        Serial.println(analog_signal);
    }

    return moisture_percentage;
}

float readTemperature(plantSensor_t *sensor)
{
    float temperature = sensor->dht.readTemperature();

    if (isnan(temperature))
    {
        Serial.println("Failed to temperature from DHT sensor!");
        return -1.0;
    }

    if (DEBUG)
    {
        Serial.print("Read temperature: ");
        Serial.println(temperature);
    }

    return temperature;
}

float readHumidity(plantSensor_t *sensor)
{
    float humidity = sensor->dht.readHumidity();

    if (isnan(humidity))
    {
        Serial.println("Failed to humidity from DHT sensor!");
        return -1.0;
    }

    if (DEBUG)
    {
        Serial.print("Read humidity: ");
        Serial.println(humidity);
    }

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

void serializePlant(plant_t *plant)
{
    Serial.println("============================");
    Serial.print("ID\t\t");
    Serial.println(plant->id);
    Serial.print("Name\t\t");
    Serial.println(plant->name);

    Serial.print("Soil moisture\t");
    Serial.println(plant->soil_moisture);
    Serial.print("Air temperature\t");
    Serial.println(plant->temperature);
    Serial.print("Air humidity\t");
    Serial.println(plant->humidity);
    Serial.print("Light level\t");
    Serial.println(plant->light);
}