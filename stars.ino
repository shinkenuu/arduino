





void setup() {

  for(byte i = 2; i < 14; i++) {
    pinMode(i, OUTPUT);
  }

}

void loop() {

  byte led_1[4] =   {5, random(03, 20), random(0, 5), random(220, 255)}; //red
  byte led_2[4] =   {6, random(30, 90), random(0, 5), random(140, 160)}; //blu

  fade(led_1[0], led_1[1], led_1[3], led_1[2]);
  fade(led_2[0], led_2[1], led_2[2], led_2[3]);
  
  fade(led_1[0], led_1[1], led_1[2], led_1[3]);
  fade(led_2[0], led_2[1], led_2[3], led_2[2]);
}

void fade(byte pin, byte delayAmount, byte from, byte to) {
  for(;from < to; from++) {
    analogWrite(pin, from);
    delay(delayAmount);
  }
}

