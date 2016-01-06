/*
  THIS SOURCE CODE ORIGINALLY INTENDED FOR USE IN UCSI UNIVERSITY ENGINEERING FINAL YEAR PURPOSES ONLY,
  NOW IS FULLY OPEN-SOURCE UNDER MIT LICENSE

  The MIT License (MIT)

  Copyright (c) 2015 AaronKow
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

/*External library for temp + humid sensor (DHT22)*/
#include <DHT.h>

/*External library for Cloud Data*/
#include <Process.h>

/*External library enables communication between Arduino and OpenWrt-Yun*/
#include <Bridge.h>

/*External library for initiate the YunServer*/
#include <YunServer.h>

/*External library for managing the connection*/
#include <YunClient.h>

YunServer server; //enabling the the Yun to listen for connected clients

/*Current Sensor Configuration*/
const int numReadings = 5;
float readings[numReadings];  
int index = 0;                 
float total = 0;             
float average = 0;
float currentValue = 0;
float initVal = 0;

/*Ultrasonic Sensor Configuration*/
const int trigPin = 2;
const int echoPin = 3;
long duration, cm;

/*Gas Sensor Configuration*/
int gasValue;

/*TempHumid Sensor Configuration*/
const int tempPin = 5;
float t,h;                         //variables for temperature sensor
DHT dht(tempPin);                  //define temperature sensor configuration

/*Cloud Data Configuration*/
String value0, value1, value2, value3, value4, value5, value6, value7;  // For sensors values
String led1, led2, led3, led4;  // For LED values
String ipAddress = "192.168.0.105:3000";    // set your ip address here
String userid = "your-user-id-here";        // set your user id here

void setup() {
  /*Http Client Setup*/
  pinMode(8, OUTPUT);              // Living Room Lights
  pinMode(9, OUTPUT);              // Bedroom Light
  pinMode(10, OUTPUT);             // Bathroom Light
  pinMode(11, OUTPUT);             // Kitchen Light
  
  /*Current Sensor Setup*/
  pinMode(0, INPUT);  
  pinMode(13,OUTPUT);  // for transmit data to Mega
  for (int thisReading = 0; thisReading < numReadings; thisReading++){
    readings[thisReading] = 0;       
  }
  
  /*Ultrasonic Sensor Setup*/
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  /*Vibration Sensor Setup*/
  pinMode(A2, INPUT);
  pinMode(A3, OUTPUT);
  
  /*Water Sensor Setup*/
  pinMode(6, INPUT);
  
  /*Buzzer Pin Setup*/
  pinMode(7, OUTPUT);
  
  /*Initiate Setup*/
  Serial.begin(115200); //Set serial baud rate to 115200 bps
  Bridge.begin();  // Initialize the Bridge communication
  server.begin();  // enabling Yun to listen for connected clients
  server.noListenOnLocalhost();    // tells the server to begin listening for incoming connections  
}

void loop() {
  /*http action*/
  YunClient client = server.accept();
  if (client.connected()) {
    Serial.println("CLIENT CONNECTED!");
    process(client); // Process request
    client.stop(); // Close connection and free resources
  }
  
  clearCloudData();
  systemStatus();
  currentSensor();
  ultrasonicSensor();
  gasSensor();
  vibrationSensor();
  TempHumidSensor();
  waterSensor();
  cloudData();
  //delay(500);
}

void process(YunClient client) {
  String command = client.readStringUntil('/'); // read the command
  if (command == "digital") { // verify if command for digital
    digitalCommand(client);
  }
}

void digitalCommand(YunClient client) {
  int pin, value;
  pin = client.parseInt(); // Read pin number

  // If the next character is a '/' it means an URL preceived
  if (client.read() == '/') {
    value = client.parseInt(); // taking value from client
    digitalWrite(pin, value);  // proceed to changes on the selected pin
  } 
  else {
    value = digitalRead(pin); // read value if no changes made
  }

  // Send feedback to client
  client.print(F("Pin D"));
  client.print(pin);
  client.print(F(" set to "));
  client.println(value);
  Serial.println(value);

  // Update datastore key with the current pin value
  String key = "D";
  key += pin;
  Bridge.put(key, String(value));
}

void clearCloudData(void){
  value0 = value1 = value2 = value3 = value4 = value5 = value6 = value7="";
}

void systemStatus(void){
  if(digitalRead(12) == 1){
    value0 += 1;
  }
  else{
    value0 += 0;
  }
}

void currentSensor(){
  total= total - readings[index];
      if(digitalRead(12) == 1){
        readings[index] = analogRead(0);
        readings[index] = (readings[index]-512)*5/1024/0.04+2.85;   // calibrate your own current sensor here
        total= total + readings[index];       
        index = index + 1;                    
        if (index >= numReadings)              
          index = 0;                           
        average = total/numReadings;
        currentValue = average;
        if (currentValue<0){
          currentValue = 0.51;    // this to ensure current stay at 0.51A if current drop below 0A
        }
        value1 += currentValue;
      }
      else{
        currentValue = 0;
        value1 += 0;
      }
      
      Serial.println(currentValue);
      
      if (currentValue >1){ //TO BE ADJUST
        digitalWrite(13, HIGH);
        buzzer(true);
        cloudData();
      }
      else{
        digitalWrite(13, LOW);
        buzzer(false);
      }
      delay(20);
}

void ultrasonicSensor(){  //require to adjust
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  cm = (duration / 29 / 2)+3;  // to centimetres

  if (cm < 20){ //eliminate data fluctuation
    buzzer(true);
    delay(500);
  }
  else{
    buzzer(false);
  }
  
  Serial.print(cm);
  Serial.println(" cm");
  value2 += cm;
  delay(20);
}

void gasSensor(){
  gasValue=(analogRead(5) * 0.01); //Read Gas value from analog 0
  Serial.println(gasValue, DEC);  //Print the value to serial port
  value3 += (gasValue);
  if(gasValue<6){
    if(gasValue == 0){
      buzzer(false);
    }
    else{
      buzzer(true);
    }
  }
  else{
    buzzer(false);
  }
  delay(20);
}

void vibrationSensor(){
  if(digitalRead(12) == 1){
    digitalWrite(A3, !digitalRead(A2));
    if (digitalRead(A2) != digitalRead(A3)){
      buzzer(true);
      delay(500);
      Serial.println("Vibrated!"); //1
      value4 += 1;
    }
    else{
      buzzer(false);
      Serial.println("No Vibration..."); //0
      value4 += 0;
    }
  }
  else{
    //Serial.println("No Signal!");
    buzzer(false);
    value4 += 0;
  }
}

void TempHumidSensor(){
  // Wait a few seconds between measurements.
  delay(20); //2000 = 2 seconds

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  h = dht.readHumidity();
  // Read temperature as Celsius
  t = dht.readTemperature();
  Serial.print("Humidity: "); 
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: "); 
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print("\n");
  value5 += t;
  value6 += h;
}

void waterSensor(){
  if (digitalRead(12) == HIGH){
    if (digitalRead(6) == LOW){
      buzzer(true);
      Serial.println("Water Status Warning!"); //1
      value7 += 0;
    }
    else{
      buzzer(false);
      Serial.println("Water Status OK"); //0
      value7 += 1;
    }
    delay(20);
  }
  else{
    //Serial.println("No Signal");
    value7 += 1;
  }
}

void buzzer(boolean sound){
  if(digitalRead(12) == 1){ //1 means security system is online
    if(sound){
      digitalWrite(7, HIGH);
      delay(100);
    }
    else{
      digitalWrite(7, LOW);
    }
  }
  else{  //disable any buzzer if system offline
    digitalWrite(7, LOW);
  }
}

void cloudData(void){
  if(value0 == "0")
  {
    value0 = "0";
    value1 = value2 = value3 = value4 = value5 = value6 = value7="";
  }

  led1 =  digitalRead(8);
  led2 =  digitalRead(9);
  led3 =  digitalRead(10);
  led4 =  digitalRead(11);
  
  Process p;
  p.runShellCommand("curl \"http://" + ipAddress + "/ledstatus?userid=" + userid + "&led1state=" + led1 + "&led2state=" + led2 + "&led3state=" + led3 + "&led4state=" + led4 + "\" -k");
  p.runShellCommand("curl \"http://" + ipAddress + "/sensordata?userid=" + userid + "&ultrasonic=" + value2 + "&current=" + value1 + "&vibration=" + value4 + "&water=" + value7 + "&gas=" + value3 + "&temp=" + value5 + "&humid=" + value6 + "&nfc=" + value0 +"\" -k");
  while(p.running()); 
  //delay(100);
}