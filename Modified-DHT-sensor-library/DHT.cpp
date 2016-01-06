/* 
  DHT library

  MIT license
  written by Adafruit Industries

  Modified by Aaron Kow for IoT Home Security Model
*/

#include "DHT.h"

DHT::DHT(uint8_t pin, uint8_t count) {
  _pin = pin;
  _count = count;
  firstreading = true;
}

void DHT::begin(void) {
  // set up the pins!
  pinMode(_pin, INPUT);
  digitalWrite(_pin, HIGH);
  _lastreadtime = 0;
}

//boolean S == Scale.  True == Farenheit; False == Celcius
float DHT::readTemperature(void) {
  float f;

  if (read()) {
      f = data[2] & 0x7F;
      f *= 256;
      f += data[3];		//>>>>>>>> improvement required<<<<<<<<
      f /= 10;
      if (data[2] & 0x80){ //negative-checker, if the and equate to 1 is true
	     f *= -1;
      }
      return f;
  }

  else{
  	return NAN;
  }
}

float DHT::readHumidity(void) {
  float f;
  if (read()) {
      f = data[0];
      f *= 256;
      f += data[1];	//>>>>>>>> improvement required<<<<<<<<
      f /= 10;
      return f;
  }

  else{
    return NAN;
  }
}

boolean DHT::read(void) {
  uint8_t laststate = HIGH;
  uint8_t counter = 0;
  uint8_t j = 0, i;
  unsigned long currenttime;

  // Check if sensor was read less than two seconds ago and return early
  // to use last reading.
  currenttime = millis();
  if (currenttime < _lastreadtime) {	//reset the last read time
    _lastreadtime = 0;
  }

  if (!firstreading && ((currenttime - _lastreadtime) < 2000)) {
    return true; // return last correct measurement
    //delay(2000 - (currenttime - _lastreadtime));
  }

  firstreading = false;
  /*
    Serial.print("Currtime: "); Serial.print(currenttime);
    Serial.print(" Lasttime: "); Serial.print(_lastreadtime);
  */
  _lastreadtime = millis();

  data[0] = data[1] = data[2] = data[3] = data[4] = 0;	//clear data
  
  // pull the pin high and wait 250 milliseconds
  digitalWrite(_pin, HIGH);
  //original delay(250);

  // now pull it low for ~20 milliseconds
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  delay(20);
  noInterrupts();			//disable interrupt for time-sensitive code
  digitalWrite(_pin, HIGH);
  delayMicroseconds(40);
  pinMode(_pin, INPUT);

  // read in timings
  for ( i=0; i< MAXTIMINGS; i++) {	//MAXTIMINGS = 85
    counter = 0;
    
  /*calibrated:response to the sensor,
    break once pin turns high.*/	
    while (digitalRead(_pin) == laststate) {	//intially laststate = high
      counter++;		
      delayMicroseconds(1);
      if (counter == 255) {	//255 is the maximum timeout
        break;
      }
    }
    laststate = digitalRead(_pin);

    if (counter == 255) break;	//if timeout, break directly

    /* 
	Condition 1: ignore first 3 transitions
	Condition 2: accept only figure divide by 2
    */
    if ((i >= 4) && (i%2 == 0)) {
      // shove each bit into the storage bytes
      data[j/8] <<= 1;	//divide by eight to ensure whole number, 0-4
      if (counter > _count) //_count here represent 60us
        data[j/8] |= 1;
      j++;
    }
  }

  interrupts();

  // check we read 40 bits and that the checksum matches
  if ((j >= 40) && 
      (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {
    return true;
  }
  else{
    return false;    
  }
}
