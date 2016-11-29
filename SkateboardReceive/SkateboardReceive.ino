#include <SPI.h>
#include "nRF24L01.h" // RF transceiver library found at https://github.com/maniacbug/RF24. http://forum.arduino.cc/index.php?topic=138663.0 is also helpful.
#include "RF24.h"
#include <FastLED.h> // WS2810B LED library found at http://fastled.io/


int16_t msg[3];
RF24 radio(9,10);
const uint64_t pipe = 0xE8E8F0F0E1LL;
boolean done = false;

byte score = 0;
#define LEDDATAPIN 3
#define RINGRINGPIN 2
#define NUMLEDS 37

CRGB leds[NUMLEDS];
CRGB gradientColor1, gradientColor2, value;

int16_t ax, ay, az;

void setup(void){
  Serial.begin(38400);
  radio.begin(); // Initialize radio
  radio.openReadingPipe(1,pipe);
  radio.startListening();
  pinMode(RINGRINGPIN, OUTPUT); // Relay pin
  digitalWrite(RINGRINGPIN, HIGH);  // Relay needs to be set to high to be normally open.
  FastLED.addLeds<WS2812B, LEDDATAPIN, GRB>(leds, NUMLEDS); // Initialize LEDs
  gradientColor1.r = 100; // Set up RGB color values for the gradient. In this case, red at the bottom, green at the top.
  gradientColor1.g = 0;
  gradientColor1.b = 0;
  gradientColor2.r = 0;
  gradientColor2.g = 100;
  gradientColor2.b = 0;
  FastLED.clear(); // Clean display just in case.
  FastLED.show();
}

void loop(void){
  if (radio.available()){
    while (!done){
      done = radio.read(msg, 6); // Receive radio signal from the skateboard

      ax = msg[0]; // Assign acceleration component values from the received array
      ay = msg[1];
      az = msg[2];
           
      Serial.print(ax); Serial.print(","); // Send raw acceleration values to Processing
      Serial.print(ay); Serial.print(",");
      Serial.println(az);

      if (Serial.available()){ // If Processing sent us the score back, read it
        readScore();
      }
    }
    done = false;
  }
  else if (Serial.available()){ // If we're not reading radio signal from the skateboard, still check if Processing sent us a score
    readScore();
  }
}

void readScore(){
  while (Serial.available()){ // Flush the serial buffer so we don't keep reading values and overring the bell
    score = Serial.read();
  }
  updateLEDs(); 
  if (score >= 254){ // Do we have a winner??
    ringBell(); // Ring the bell!
    score = 0;
    while (Serial.available()){ // Flush the serial buffer in case we got more reports of winning while we were ringing. Again, to prevent overringing
      score = Serial.read();
    }
  }
}

void updateLEDs(){
  FastLED.clear();
  for (int j = 0; j < NUMLEDS * score / 254; j++){ // Iterate from LED 0 up to an index determined by the score.
    lerpRGB(gradientColor1, gradientColor2, j * 1.0 / NUMLEDS); // Do color cradient
    leds[j] = value; // Assign this LED to the gradiated color
  }
  FastLED.show();
}

void ringBell(){ // Ring the bell!!!
  for (int i = 0; i < 6; i ++){ // DING DING DING
    digitalWrite(RINGRINGPIN, LOW); // These relay modules take a LOW signal to close the switch and a HIGH signal to open.
    delay(50);
    digitalWrite(RINGRINGPIN, HIGH);
    delay(250);
  }
}

void lerpRGB(CRGB c0, CRGB c1, float alpha){ // Naive and sloppy color lerper, but is fast and works for this purpose.
  value.r = (c1.r - c0.r)*alpha + c0.r;
  value.g = (c1.g - c0.g)*alpha + c0.g;
  value.b = (c1.b - c0.b)*alpha + c0.b;
}
