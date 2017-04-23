/*
 * Example to demonstrate thread definition, semaphores, and thread sleep.
 */
#include <FreeRTOS_AVR.h>
//#include <DHT.h>

// The LED is attached to pin 13 on Arduino.
const uint8_t LED_PIN = 13;
//SemaphoreHandle_t feedbackSemaphore;


//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//const uint8_t DHT_PIN = 3;
//DHT dht(DHT_PIN, DHTTYPE);
//SemaphoreHandle_t dhtSemaphore;

QueueHandle_t heatQueue = 0;

//----------------------------------------------------
/*
 * ReadDHT: 
 * wait for DHT signal cleaup (2000) millisec,
 * read DHT signals (~250)millisec
 * give dht_semaphore
 */
// Declare the thread function for DHT signal reading.
static void ReadDHT(void* pvParameters) {

  float humidity = -1;
  float temperature = -1;
  float heatIndex = -1;
  
  while (1) {
    
    // Sleep for 2000 milliseconds for DHT signal cleanup
    vTaskDelay(2 * configTICK_RATE_HZ);

    temperature = random(25,38); //dht.readTemperature();
    humidity = random(40,50); //dht.readHumidity();

    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temperature)) {
      //Flag reading error
      heatIndex = -1;
      //Send heat index through heatQueue and wait 2000 milliseconds till a task receive it
      xQueueSend(heatQueue, &heatIndex, 2 * configTICK_RATE_HZ);
    }
    else {
      //Compute heat index based on temperature and humidity read
      heatIndex = humidity / temperature; //dht.computeHeatIndex(temperature, humidity, false);
      //Send heat index through heatQueue and wait 2000 milliseconds till a task receive it
      xQueueSend(heatQueue, &heatIndex, 2 * configTICK_RATE_HZ);
    }
  }
}

//------------------------------------------------------------------------------
/*
 * Thread 3, turn the LED on and signal thread 1 to turn the LED off.
 */
// Declare the thread function for thread 2.
static void FeedbackThread(void* pvParameters) {

  float heatIndex;
  while (1) {
    //If received. If not, snooze 2 secs
    if(xQueueReceive(heatQueue, &heatIndex, 2 * configTICK_RATE_HZ)) {
      Serial.print("HeatIndex ");
      Serial.println(heatIndex);
    }    
  }
}

//------------------------------------------------------------------------------
void setup() {
  portBASE_TYPE s1, s2;

  Serial.begin(9600);
  //dht.begin();
  pinMode(LED_PIN, OUTPUT);

  heatQueue = xQueueCreate(1, sizeof(float));
  //dhtSemaphore = vSemaphoreCreateMutex();
  

  if (/*dhtSemaphore == NULL ||*/  heatQueue == NULL) {
    Serial.print("Problem with semaphore or queue");
    while(1);
  }
  
  // create task at priority one
  s1 = xTaskCreate(ReadDHT, (signed char*)"ReadDHT", 3 * sizeof(float) + 200, NULL, tskIDLE_PRIORITY + 1, NULL);
  // create task at priority three
  s2 = xTaskCreate(FeedbackThread, (signed char*)"FeedbackThread", sizeof(float) + 200 * 2, NULL, tskIDLE_PRIORITY + 2, NULL);


  if(s1 != pdPASS || s2 != pdPASS /*|| s3 != pdPASS*/) {
    Serial.print("task creation problem ");
    if(s1 != pdPASS) Serial.print("s1");
    if(s2 != pdPASS) Serial.print("s2");
    //if(s3 != pdPASS) Serial.print("s3");
    while(1);
  }
  
  // start scheduler
  vTaskStartScheduler();
  
  Serial.println("Die");
  while(1);
}

//------------------------------------------------------------------------------
// WARNING idle loop has a very small stack (configMINIMAL_STACK_SIZE)
// loop must never block
void loop() {
  // Not used.
}
