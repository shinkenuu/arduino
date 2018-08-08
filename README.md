# Arduino Real Time - A implementation of FreeRTOS in Arduino

This project was designed with the purpose of testing [FreeRTOS for AVR](https://sourceforge.net/projects/avrfreertos/) using semaphores (shared resource), queues (message from thread to thread) and thread sync.
Thus, it fakes a busy RX resource while the user is trying to send messages to it (via TX). Everything is logged in the serial monitor.

## The circuit

You can find the hardware design used in the image [circuit.png](https://github.com/shinkenuu/arduino/tree/master/ArduinoRealTime/circuit.png)
Please note that I used [1-Way Photo Resistor](http://www.dx.com/p/1-way-photo-resistor-sensor-module-for-arduino-blue-151310) instead of the photoresistor and potentiometer.
Please also note that the white LED must be directed to the photoresistor (as I tried to show in the image)

## What does it uses

The software uses a semaphore (rxSemp), a queue (rxQueue), an scheduler with tasks (s1, s2, s3) priorities.

* [Semaphore](https://en.wikipedia.org/wiki/Semaphore_%28programming%29) is used to indicate that a resource is being used - in this software, the RX resource -. If a thread takes the semaphore, it stays with that thread until it gives it up. After a thread has taken a semaphore is known for all other threads that tries to take it that it's refered resource is being used (red signal). It the semaphore is taken the refered resource is available (green signal).

* [Queue](https://en.wikipedia.org/wiki/Queue_%28abstract_data_type%29) is used to send data through a organized data structure keep the injection order (FIFO) that other threads have access to (instead of an single datatype global variable) - in this example, the signal read from RX_PIN by the ListenThread -.

* [Thread](https://en.wikipedia.org/wiki/Thread_%28computing%29) is used to take care of a minimal and single matter - in this case, "Listen" to at is at RX_PIN, "Busy" to fake a busy resource, "Feedback" to update the logs -. Each is scheduled by the TaskScheduler with a priority. The higher the priority number, the higher the priority of this thread to used the processor.

## How does it works

The software consists of 3 threads, each sharing the processor cooperatively (using, giving it up, receving it back)

1. `ListenThread`
	Asks for the rx semaphore to be able to read what is in RX_PIN. If the semaphore is taken, it read what is at RX_PIN and send it through rxQueue so that our feedback thread can print it. If the semaphore is not taken within hald second, the message "-1" is put in the queue to indicate that the rxSemaphore was not available. Then it gaves up the processor.

2. `BusyThread`
	Fakes another process using the RX resource by a short random time interval (between 1 and 4 seconds). For each half second this thread has 1 in 5 chances of taking the rx semaphore, thus setting the TX_PIN (green LED) and locking the main resource that our 2 other threads depend on. If the semaphore is not taken within hald second, it gives up the processor.

3. `FeedbackThread`
	Responsable to logging what the RX received by reading the rxQueue and logging it into the serial monitor. After printing, it gives up the processor.


