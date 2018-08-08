/*
 * Example to demonstrate thread definition, semaphores, queue and thread sleep.
 */
#include <FreeRTOS_AVR.h>

#define MIN_DELAY_MOD 1
#define MAX_DELAY_MOD 4

// The LED is attached to pin 13 on Arduino and is used by FreeRTOS
const uint8_t TX_PIN = 3;
const uint8_t RX_PIN = 2;

SemaphoreHandle_t rxSemp = NULL;
QueueHandle_t rxQueue = 0;

/*
 * ListenThread: 
 * Reads the signal from the photo-resistor at RX_PIN
 * and send it through the rxQueue
 */
static void ListenThread(void* pvParameters) {

  pinMode(RX_PIN, INPUT);
  int msg;
  
  while (1) {

      //Try to take the semaphore. If couldnt, wait for it
      //as much as this thread can stay at the processor
      if(xSemaphoreTake(rxSemp, 0.5 * configTICK_RATE_HZ) == pdTRUE) {
        //If taken
        //read rx
        msg = digitalRead(RX_PIN); // possible values = {0, 1}

        //Release the resource
        xSemaphoreGive(rxSemp);        
      }
      else {
        //If not taken
        //Signal an error (-1)
        msg = -1;
      }
  
      //send read signal through queue, keeping the signal for 500 milliseconds
      xQueueSend(rxQueue, &msg, 0.5 * configTICK_RATE_HZ);
            
      //Give processing time so BusyThread can be able to take the semaphore
      vTaskDelay(0.5 * configTICK_RATE_HZ);
  }
}


/*
 * BusyThread: 
 * Fake a busy thread using the RX resource
 */
static void BusyThread(void* pvParameters) {

  pinMode(TX_PIN, OUTPUT);
  digitalWrite(TX_PIN, LOW);
  
  while (1) {
        
    //If busy
    //1 in each 5 chances of being busy
    if(!random(0, 5)) {
      
      //Wait for as much as the period
      //the listenchannel thread can use the semaphore
      //making sure the semaphore will be ours
      if(xSemaphoreTake(rxSemp, 0.5 * configTICK_RATE_HZ) == pdTRUE) {

        //Signal the fake thread is ON
        digitalWrite(TX_PIN, HIGH);
        Serial.println("Resource is busy. Hold on...");
  
        //hold the semaphor for a while (1~4 secs)
        vTaskDelay(random(MIN_DELAY_MOD, MAX_DELAY_MOD) * configTICK_RATE_HZ);
          
        //Signal the fake thread is OFF
        digitalWrite(TX_PIN, LOW);
        Serial.println("OK, clear");
        
        //Gives the tx semaphore listen at the channel
        xSemaphoreGive(rxSemp);
      }     
    }

    //Give processing time so ListenChannel thread can be able to take the semaphore
    vTaskDelay(0.5 * configTICK_RATE_HZ);
  }
}

/*
 * FeedbackThread
 * Logs what is at the rxQueue
 */
static void FeedbackThread(void* pvParameters) {

  int signalReceived;
  while (1) {
    //If received. Keeps trying for MAX_DELAY_MOD (4) seconds    
    if(xQueueReceive(rxQueue, &signalReceived, MAX_DELAY_MOD * configTICK_RATE_HZ)) {
      Serial.print("Received ");
      Serial.println(signalReceived);
    }    
    else {
      Serial.println("Received nothing");
    }
  }
}

//------------------------------------------------------------------------------
void setup() {
  portBASE_TYPE s1, s2, s3;

  Serial.begin(9600);

  rxQueue = xQueueCreate(1, sizeof(int));
  rxSemp = xSemaphoreCreateMutex();  

  if (rxSemp == NULL || rxQueue == NULL) {
    Serial.print("Problem with semaphore or queue");
    while(1);
  }

  if (xSemaphoreTake(rxSemp, 0) == pdTRUE) {
    xSemaphoreGive(rxSemp);
  } else {
    Serial.print("Problem taking rxSemp");
    while(1);
  }
  
  s1 = xTaskCreate(ListenThread, (signed char*)"ListenThread", sizeof(int) + 200, NULL, 3, NULL);
  s2 = xTaskCreate(BusyThread, (signed char*)"BusyThread", 200, NULL, 1, NULL);
  s3 = xTaskCreate(FeedbackThread, (signed char*)"FeedbackThread", sizeof(int) + 200, NULL, 2, NULL);

  if(s1 != pdPASS 
  || s2 != pdPASS 
  || s3 != pdPASS) {
    Serial.print("COULD_NOT_ALLOCATE_REQUIRED_MEMORY for ");
    if(s1 != pdPASS) Serial.print("s1");
    if(s2 != pdPASS) Serial.print("s2");
    if(s3 != pdPASS) Serial.print("s3");
    while(1);
  }

  Serial.println("All set. Lets go!");
  // start scheduler
  vTaskStartScheduler();
  
  Serial.println("Die");
  while(1);
}

void loop() {
  // Not used.
}
