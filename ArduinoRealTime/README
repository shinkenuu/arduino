# Arduino Real Time - A implementation of FreeRTOS in Arduino

This project was designed with the purpose of testing FreeRTOS for AVR using semaphores (shared resource), queues (message from thread to thread) and thread sync.
Thus, it fakes a busy RX resource while the user is trying to send messages to it (via TX). Everything is logged in the serial monitor.

## The circuit

You can find the hardware design used in the image [circuit.png](https://github.com/shinkenuu/arduino/tree/master/ArduinoRealTime/circuit.png)
Please note that I used [1-Way Photo Resistor](http://www.dx.com/p/1-way-photo-resistor-sensor-module-for-arduino-blue-151310) for instead of the photoresistor and potentiometer.
Please also note that the white LED must be directed to the photoresistor (as I tried to show in the image)

## How does it works

The software consists of 3 threads, each sharing the processor between cooperatively (using, giving it up, receving it back)
1. ListenThread
	Asks for the rx semaphore to be able to read what is in RX. If the semaphore is taken, it read what is at out RX pin and send it through rxQueue so that our feedback thread can print it. If the semaphore is not taken within hald second, it gives up the processor.

2. BusyThread 
	Fakes another process using the RX resource by a short random time interval (between 1 and 4 seconds). For each half second this thread has 1 in 5 chances of taking the rx semaphore, thus locking the main resource that our 2 other threads depend on. If the semaphore is not taken within hald second, it gives up the processor.

3. FeedbackThread
	Responsable to logging what the RX received by reading the rxQueue and logging it into the serial monitor. If the RX resource is not responding to this process (the busy thread took control of it), the message "Received nothing" is printed. After printing, it gives up the processor.


