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


//pin 3 to control relay power for Ultrasonic and gas sensor
//pin 4 to power vibration sensor
//pin 5 to power Temp + Humid sensor
//pin 6 to power Water sensor
//pin 7 to show NFC-System Status Red Light
//pin 8 to show NFC-System Status Green Light
//pin 12 to notify Yun about System Status
//pin 13 to receive current status from Yun

/* NFC Configuration source code from dfRobot Wiki, link: http://goo.gl/qfvi4e */
const unsigned char wake[16]={
  0x55, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};//wake up NFC module
const unsigned char firmware[9]={
  0x00, 0x00, 0xFF, 0x02, 0xFE, 0xD4, 0x02, 0x2A, 0x00};
const unsigned char tag[11]={
  0x00, 0x00, 0xFF, 0x04, 0xFC, 0xD4, 0x4A, 0x01, 0x00, 0xE1, 0x00};//detecting tag command
const unsigned char std_ACK[25] = {
  0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x0C,
  0xF4, 0xD5, 0x4B, 0x01, 0x01, 0x00, 0x04, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x00};
unsigned char old_id[5];
const unsigned char samConfig[10]={
  0x00, 0x00, 0xFF, 0x03, 0xFD, 0xD4, 0x14, 0x01, 0x17, 0x00};
  
const unsigned char myCard[25]={
  /* Set your NFC here */
  // Example of my phone NFC below:
  // My phone NFC:   0 0 FF 0 FF 0 0 0 FF 11 EF D5 4B 1 1 0 4 25 91 5 4 3 2 1 79 
  0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x11,
  0xEF, 0xD5, 0x4B, 0x01, 0x01, 0x00, 0x04, 0x25, 0x91, 0x05,
  0x04, 0x03, 0x02, 0x01, 0x79};
 
unsigned char receive_ACK[25];//Command receiving buffer
//int inByte = 0;               //incoming serial byte buffer
 
/*External include for NFC*/
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#define print1Byte(args) Serial3.write(args)
#define print1lnByte(args)  Serial3.write(args),Serial3.println()
#else
#include "WProgram.h"
#define print1Byte(args) Serial3.print(args,BYTE)
#define print1lnByte(args)  Serial3.println(args,BYTE)
#endif

void setup() {
  /*NFC Setup*/
  Serial3.begin(115200);    //open Serial3 with device
  //while (!Serial);
  wake_card();
  SAMConfig();
  delay(100);
  read_ACK(15);
  delay(100);
  display(15);
  Serial.println("Read Firmware");
  firmware_version();
  delay(100);
  read_ACK(19);
  delay(100);
  display(19);
  
  /*Data Transmission to Yun Setup*/
  pinMode(12, OUTPUT);  //Tell Yun where system Logged in or out
  digitalWrite(12, HIGH);
  
  /*System Status Setup*/
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  digitalWrite(7, LOW);
  digitalWrite(8, HIGH);
  
  /*General Setup*/
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(13, INPUT);
  digitalWrite(3, LOW);  //to control relay
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  Serial.begin(115200);   // open serial with PC
}

void loop() {
  //Serial.println(digitalRead(13));
  checkStatus();
  nfc();
}

void nfc(){
  send_tag();
  delay(100); 
  read_ACK(25);
  delay(100);
  Serial.println("Display Tag:");
  display (25);
  checkID();
  delay(200);
  //delay(2500);
  //copy_id ();
}

void checkID(void){
  int j;
  for (j=0; j<25; j++){
    if (receive_ACK[j] == myCard[j]){
      //proceed for validation
    }
    
    else{
      Serial.println("Scanning for Card...");
      break;
    }
  }
  if (j==25){
    Serial.println("Card Recognized!");
    digitalWrite(12, !digitalRead(12));
    digitalWrite(7, !digitalRead(7));
    digitalWrite(8, !digitalRead(8));
    if(digitalRead(12) == HIGH){
      Serial.println("System Online");  //Security system offline
    }
    else{
      Serial.println("System Offline");  
    }
  }
}
 
/*void copy_id (void) 
{//save old id
  int ai, oi;
  for (oi=0, ai=19; oi<5; oi++,ai++) {
    old_id[oi] = receive_ACK[ai];
  }
}*/
 
  
char cmp_id (void) 
{//return true if find id is old
  int ai, oi;
  for (oi=0,ai=19; oi<5; oi++,ai++) {
    if (old_id[oi] != receive_ACK[ai])
      return 0;
  }
  return 1;
}
 
 
int test_ACK (void) 
{// return true if receive_ACK accord with std_ACK
  int i;
  for (i=0; i<19; i++) {
    if (receive_ACK[i] != std_ACK[i])
      return 0;
  }
  return 1;
}
 
 
void send_id (void) 
{//send id to PC
  int i;
  Serial.print ("ID: ");
  for (i=19; i<= 23; i++) {
    Serial.print (receive_ACK[i], HEX);
    Serial.print (" ");
  }
  Serial.println ();
}
 
 
void UART1_Send_Byte(unsigned char command_data)
{//send byte to device
  print1Byte(command_data);
#if defined(ARDUINO) && ARDUINO >= 100
  Serial3.flush();// complete the transmission of outgoing serial data 
#endif
} 
 
 
void UART_Send_Byte(unsigned char command_data)
{//send byte to PC
  Serial.print(command_data,HEX);
  Serial.print(" ");
} 
 
 
void read_ACK(unsigned char temp)
{//read ACK into reveive_ACK[]
  unsigned char i;
  for(i=0;i<temp;i++) {
    receive_ACK[i]= Serial3.read();
  }
}
 
 
void wake_card(void)
{//send wake[] to device
  unsigned char i;
  for(i=0;i<16;i++){ //send command
    UART1_Send_Byte(wake[i]);
    //Serial.print(wake[i], HEX);
  }
}
 
 
void firmware_version(void)
{//send fireware[] to device
  unsigned char i;
  for(i=0;i<9;i++) //send command
    UART1_Send_Byte(firmware[i]);
}
 
 
void send_tag(void)
{//send tag[] to device
  unsigned char i;
  for(i=0;i<11;i++) //send command
    UART1_Send_Byte(tag[i]);
}
 
 
void display(unsigned char tem)
{//send receive_ACK[] to PC
  unsigned char i;
  for(i=0;i<tem;i++) //send command
    UART_Send_Byte(receive_ACK[i]);
  Serial.println();
}

void SAMConfig(void)
{
  unsigned char i;
  for(i=0;i<10;i++){ //send command
    UART1_Send_Byte(samConfig[i]);
  }
}

void checkStatus(void){
  if(digitalRead(12) == 0){ //system offline
    digitalWrite(3, HIGH);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
  }
  else{  //system online
    if(digitalRead(13) == 1){ //check current status
      digitalWrite(3, HIGH);
      digitalWrite(4, LOW);
      digitalWrite(5, LOW);
      digitalWrite(6, LOW);
    }
    
    else{ //system back online
      digitalWrite(3, LOW);
      digitalWrite(4, HIGH);
      digitalWrite(5, HIGH);
      digitalWrite(6, HIGH);
    }
  }
}
