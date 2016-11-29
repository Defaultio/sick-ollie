#include <SPI.h>
#include "nRF24L01.h" // RF transceiver library found at https://github.com/maniacbug/RF24. http://forum.arduino.cc/index.php?topic=138663.0 is also helpful.
#include "RF24.h"
#include "I2Cdev.h" // Accelerometer library found at https://github.com/jrowberg/i2cdevlib
#include "MPU6050.h"

int16_t msg[3];
RF24 radio(9,10);
const uint64_t pipe = 0xE8E8F0F0E1LL;

MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;

void setup(void){
  Serial.begin(38400);
  radio.begin(); // Initialize radio
  radio.openWritingPipe(pipe);
  Wire.begin();
  accelgyro.initialize(); // Initialize accelerometer
  accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_16); // Set accelerometer sensitivity to +/- 16 G's (default is only +/- 2)
}

void loop(void){
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz); // Read accelerometer values
  msg[0] = ax; // Assign values to array
  msg[1] = ay;
  msg[2] = az;

  radio.write(msg, 6); // Transmit array
}


