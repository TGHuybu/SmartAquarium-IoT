#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "pitches.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <math.h>

#define W_ULTRASONIC_TRIG D0
#define W_ULTRASONIC_ECHO D1
#define A_ULTRASONIC_TRIG D2
#define A_ULTRASONIC_ECHO D7
#define SERVO D4
#define LCD_SCL D5
#define LCD_SDA D6
#define TEMP D3
#define BUZZER D8

// Set the I2C address for your specific LCD module
#define LCD_I2C_ADDR 0x27

// Set the number of columns and rows on the LCD
#define LCD_COLUMNS 16
#define LCD_ROWS 2

// Servo
Servo servo;
// Create an LCD object with the I2C address
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, LCD_COLUMNS, LCD_ROWS);
// Temperature sensor
OneWire oneWire(TEMP);
DallasTemperature tempSensor(&oneWire);

/* Other variables */
int noteIndex = 0;
int lcdCurrentScreen = 1;
float currentTemperature = 0;
long waterHeight = 0;
String currentSong = "null";

const char* ssid = "Simba 2.4";
const char* password = "19091970";
//***Set server***
const char* mqttServer = "broker.hivemq.com"; 
int port = 1883;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

/* Timers */
const int servoTimer = 5000;
const int lcdTimer = 2500;
const int buzzerTimer = 0;
int servoStart = -1, servoEnd = -1;
int lcdStart = -1, lcdEnd = -1;
bool servoState = false;

void setup() {
  // Init basic
  Serial.begin(9600);

  /* WiFi connection */
  WiFi.begin(ssid, password);

  /* LCD */
  // Initialize the Wire library with D5 (SCL) and D6 (SDA)
  Wire.begin(LCD_SCL, LCD_SDA);

  lcd.begin(LCD_COLUMNS, LCD_ROWS);

  /* Servo */
  // Init servo
  servo.attach(SERVO);

  /* Temperature sensor */
  tempSensor.begin();

  /* Ultrasonic sensors */
  pinMode(W_ULTRASONIC_TRIG, OUTPUT);
  pinMode(W_ULTRASONIC_ECHO, INPUT);
  pinMode(A_ULTRASONIC_TRIG, OUTPUT);
  pinMode(A_ULTRASONIC_ECHO, INPUT);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED)
    Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Connection successful");
      break;
    }
  }
  // MQTT
  mqttClient.setServer(mqttServer, port);
  mqttClient.setCallback(callback);
  mqttClient.setKeepAlive( 90 );
  if(!mqttClient.connected()) {
    mqttConnect();
  }
  mqttClient.loop();

  char buffer[50];
  String msg = String(currentTemperature) + "," + String(getUltrasonicDist(W_ULTRASONIC_TRIG, W_ULTRASONIC_ECHO));
  msg.toCharArray(buffer, 50);
  mqttClient.publish("fishtank/sensordata", buffer);
  
  // Time buzzer
  if (currentSong == "jingle_bell")
    buzzerSong1();
  else if (currentSong == "twinkle_star")
    buzzerSong2();
  // Wait till buzzer has finished playing music to update the temperature (since the process is slow, it will pause the music)
  if (noteIndex == 0) {
    updateTemperature();
  }

  // Time servo
  if (servoStart == -1 && servoEnd == -1) {
    servoStart = servoEnd = millis();
  }
  else {
    servoEnd = millis();
    if (servoEnd - servoStart >= servoTimer) {
      
      controlServo(servoState);
      servoStart = servoEnd = -1;
    }
  }

  // LCD
  if (getUltrasonicDist(A_ULTRASONIC_TRIG, A_ULTRASONIC_ECHO) <= 15) {
    // Turn on the backlight
    lcd.backlight();
  }
  else {
    // Turn off the backlight
    lcd.noBacklight();
  }
  if (lcdStart == -1 && lcdEnd == -1) {
    lcdStart = lcdEnd = millis();
  }
  else {
    lcdEnd = millis();
    if (lcdEnd - lcdStart >= lcdTimer) {
      displayLCD();
      lcdStart = lcdEnd = -1;
    }
  }
  delay(50);
}

void mqttConnect() {
  while(!mqttClient.connected()) {
    Serial.println("Attemping MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if(mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");

      //***Subscribe all topic you need***
      mqttClient.subscribe("fishtank/level_lcd");
      mqttClient.subscribe("fishtank/music");
      mqttClient.subscribe("fishtank/feeder");
    }
    else {
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

//MQTT Receiver
void callback(char* topic, byte* message, unsigned int length) {
  String strMsg;
  for(int i=0; i<length; i++) {
    strMsg += (char)message[i];
  }

  String topicc(topic);

  // Serial.println(topicc.equals(topic1));
  //***Code here to process the received package***
  if (topicc.equals("fishtank/level_lcd"))
    setWaterHeight(atol(strMsg.c_str()));
  else if (topicc.equals("fishtank/music")) {
    Serial.print("Music: ");
    Serial.println(strMsg);
    setSongName(strMsg);
  }
  else if (topicc.equals("fishtank/feeder")) {
    Serial.print("IsFeederOpen: ");
    Serial.println(strMsg);
    if (strMsg == "true") {
      servoState = true;
    }
    else {
      servoState = false;
    }
  }
}

void controlServo(bool toOpen) {
  if (toOpen)
    servo.write(0);
  else if (!toOpen)
    servo.write(180);
}

void setSongName(String songName) {
  currentSong = songName;
  noteIndex = 0;
}

void buzzerSong1() {
  // notes in the melody:
  int melody[] = {
    NOTE_E5, NOTE_E5, NOTE_E5,
    NOTE_E5, NOTE_E5, NOTE_E5,
    NOTE_E5, NOTE_G5, NOTE_C5, NOTE_D5,
    NOTE_E5,
    NOTE_F5, NOTE_F5, NOTE_F5, NOTE_F5,
    NOTE_F5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5,
    NOTE_E5, NOTE_D5, NOTE_D5, NOTE_E5,
    NOTE_D5, NOTE_G5
  };
  int noteDurations[] = {
    8, 8, 4,
    8, 8, 4,
    8, 8, 8, 8,
    2,
    8, 8, 8, 8,
    8, 8, 8, 16, 16,
    8, 8, 8, 8,
    4, 4
  };

  // Play the melody
  if (noteIndex < sizeof(melody) / sizeof(melody[0])) {
    int noteDuration = 1000 / noteDurations[noteIndex];
    tone(BUZZER, melody[noteIndex], noteDuration);
    noteIndex++;

    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    
    // stop the tone playing:
    noTone(BUZZER);
  } else {
    // Reset the index to play the melody again
    noteIndex = 0;
  }
}

void buzzerSong2() {
  // notes in the melody:
  int melody[] = {
    NOTE_C4, NOTE_C4, NOTE_G4, NOTE_G4,
    NOTE_A4, NOTE_A4, NOTE_G4, 0,
    NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4,
    NOTE_D4, NOTE_D4, NOTE_C4, 0,
    NOTE_G4, NOTE_G4, NOTE_F4, NOTE_F4,
    NOTE_E4, NOTE_E4, NOTE_D4, 0,
    NOTE_G4, NOTE_G4, NOTE_F4, NOTE_F4,
    NOTE_E4, NOTE_E4, NOTE_D4, 0,
    NOTE_C4, NOTE_C4, NOTE_G4, NOTE_G4,
    NOTE_A4, NOTE_A4, NOTE_G4, 0,
    NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4,
    NOTE_D4, NOTE_D4, NOTE_C4, 0
  };
  // Note durations: 4 = quarter note, 8 = eighth note, etc.
  int noteDurations[] = {
    4, 4, 4, 4,
    4, 4, 2, 4,
    4, 4, 4, 4,
    4, 4, 2, 4,
    4, 4, 4, 4,
    4, 4, 2, 4,
    4, 4, 4, 4,
    4, 4, 2, 4,
    4, 4, 4, 4,
    4, 4, 2, 4,
    4, 4, 4, 4,
    4, 4, 2, 4
  };

  // Play the melody
  if (noteIndex < sizeof(melody) / sizeof(melody[0])) {
    int noteDuration = 1000 / noteDurations[noteIndex];
    tone(BUZZER, melody[noteIndex], noteDuration);
    noteIndex++;

    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    
    // stop the tone playing:
    noTone(BUZZER);
  } else {
    // Reset the index to play the melody again
    noteIndex = 0;
  }
}

void updateTemperature() {
  tempSensor.requestTemperatures(); 
  currentTemperature = tempSensor.getTempCByIndex(0);
}

long getUltrasonicDist(int trig_pin, int echo_pin) {
  digitalWrite(trig_pin, LOW);
  delayMicroseconds(2);
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);

  long duration = pulseIn(echo_pin, HIGH);

  long distanceCm = duration * 0.034 / 2;

  return distanceCm;
}

void setWaterHeight(long height) {
  waterHeight = height;
}

void displayLCD() {
  lcd.clear();
  if (lcdCurrentScreen % 3 == 1) {
    lcd.setCursor(0, 0); // Set the cursor to the first column and first row
    lcd.print("Temp C:");
    lcd.setCursor(0, 1); // Set the cursor to the first column and 2nd row
    lcd.print(currentTemperature);
  }
  else if (lcdCurrentScreen % 3 == 2) {
    lcd.setCursor(0, 0); // Set the cursor to the first column and first row
    lcd.print("Water level:");
    lcd.setCursor(0, 1); // Set the cursor to the first column and 2nd row
    lcd.print(waterHeight);
    lcd.print(" cm");
  }
  else {
    lcd.setCursor(0, 0); // Set the cursor to the first column and first row
    lcd.print("Now playing: ");
    lcd.setCursor(0, 1); // Set the cursor to the first column and 2nd row
    lcd.print(currentSong);
  }
  if (lcdCurrentScreen >= 3) lcdCurrentScreen = 1;
  else lcdCurrentScreen++;
}
