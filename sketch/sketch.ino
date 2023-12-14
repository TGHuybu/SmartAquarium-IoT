#include <dummy.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int trigWaterPin = D0;
const int echoWaterPin = D1;
const int trigAirPin = D2;
const int echoAirPin = D3;
const int servoPin = D4;
const int sdaLCD = D5;
const int sclLCD = D6;
const int waterTempPin = D7;
const int buzzerPin = D8;

long getDistanceWater(){
  digitalWrite(trigWaterPin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigWaterPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigWaterPin,LOW);

  long duration = pulseIn(echoWaterPin,HIGH);

  long distanceCm = dutarion * 0.034 / 2;

  return distanceCm;
}

long getDistanceAir(){
  digitalWrite(trigAirPin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigAirPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigAirPin,LOW);

  long duration = pulseIn(echoAirPin,HIGH);

  long distanceCm = dutarion * 0.034 / 2;

  return distanceCm;
}


void setup(){
  Serial.begin(9600);
  
  pinMode(trigWaterPin,OUTPUT);
  pinMode(echoWaterPin,INPUT);
  pinMode(trigAirPin,OUTPUT);
  pinMode(echoAirPin,INPUT);
}


void loop(){

}