/* 
  DHT library

  MIT license
  written by Adafruit Industries

  Modified by Aaron Kow for IoT Home Security Model
*/

#ifndef DHT_H
#define DHT_H
#if ARDUINO >= 100

#include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#define MAXTIMINGS 85

class DHT {
 private:
  uint8_t data[6];
  uint8_t _pin, _count;
  unsigned long _lastreadtime;
  boolean firstreading;

 public:
  DHT(uint8_t pin, uint8_t count=6);
  void begin(void);
  float readTemperature(void);
  float readHumidity(void);
  boolean read(void);
};
#endif
