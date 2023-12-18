#include <Servo.h>
#include <Wire.h>
#include <ESP8266WiFi.h> //wifi for esp8266
#include <LiquidCrystal_I2C.h>
// #include "pitches.h" //comment vi ko su dung dc
#include <OneWire.h>
#include <DallasTemperature.h>
#include <RTClib.h>

#define W_ULTRASONIC_TRIG D0
#define W_ULTRASONIC_ECHO D1
#define A_ULTRASONIC_TRIG D2
#define A_ULTRASONIC_ECHO D3
#define SERVO D4
#define LCD_SCL D5
#define LCD_SDA D6
#define TEMP D7
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

// Set time to open the servo
int targetHour = 13;
int targetMinute = 30;
RTC_DS3231 rtc;  // Use RTC_DS1307 for DS1307 RTC module

/* Other variables */
int noteIndex = 0;
unsigned long lastNoteTime = 0;

/* Timers */
const int servoTimer = 5000;
const int lcdTimer = 1000;
const int buzzerTimer = 0;
int servoStart = -1, servoEnd = -1;
int lcdStart = -1, lcdEnd = -1;

void setup() {
  // Init basic
  Serial.begin(9600);

  /* LCD */
  // Initialize the Wire library with D5 (SCL) and D6 (SDA)
  Wire.begin(LCD_SCL, LCD_SDA);
  // Set up rtc 
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }


  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  // Turn on the backlight (if available on your LCD)
  lcd.backlight();

  /* Servo */
  // Init servo
  servo.attach(SERVO);

  /* Temperature sensor */
  tempSensor.begin();
}

void loop() {
  DateTime now = rtc.now();
  // Check if it's time to run the servo
  if (now.hour() == targetHour && now.minute() == targetMinute) {
    controlServo();
  }

  // Time buzzer
  controlBuzzer();

  // Time servo
  if (servoStart == -1 && servoEnd == -1) {
    servoStart = servoEnd = millis();
  }
  else {
    servoEnd = millis();
    if (servoEnd - servoStart >= servoTimer) {
      controlServo();
      servoStart = servoEnd = -1;
    }
  }

  Serial.println(lcdEnd - lcdStart);
  // Time temperature sensor
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

  delay(10);
}


void controlServo() {
  
  if (servo.read() == 0)
    servo.write(180);
  else
    servo.write(0);
}

void controlBuzzer() {
  // Melody for "Twinkle, Twinkle, Little Star"
  int melody[] = { NOTE_C4, NOTE_C4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_G4, 0,
                   NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4, NOTE_C4, 0,
                   NOTE_G4, NOTE_G4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4, 0,
                   NOTE_G4, NOTE_G4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4, 0,
                   NOTE_C4, NOTE_C4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_G4, 0,
                   NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4, NOTE_C4, 0 };

  // Durations for each note (in milliseconds)
  int noteDurations[] = { 400, 400, 400, 400, 400, 400, 800, 400,
                          400, 400, 400, 400, 400, 400, 800, 400,
                          400, 400, 400, 400, 400, 400, 800, 400,
                          400, 400, 400, 400, 400, 400, 800, 400,
                          400, 400, 400, 400, 400, 400, 800, 400,
                          400, 400, 400, 400, 400, 400, 800, 400 };

  // Index to keep track of the current note

  // Volume control factor (adjust as needed, 0.5 is half volume)
  const float volumeFactor = 0.5;

  // Play the melody
  if (noteIndex < sizeof(melody) / sizeof(melody[0])) {
    if (melody[noteIndex] != 0) {
      // Adjust amplitude for lower volume
      int amplitudeAdjusted = int(volumeFactor * 255.0);
      tone(BUZZER, melody[noteIndex], noteDurations[noteIndex]);
      lastNoteTime = millis();
      noteIndex++;
    } else {
      // Rest
      unsigned long restTime = noteDurations[noteIndex];
      if (millis() - lastNoteTime >= restTime) {
        noteIndex++;
      }
    }
  } else {
    // Reset the index to play the melody again
    noteIndex = 0;
  }
}

float getTemperatureC() {
  tempSensor.requestTemperatures(); 
  return tempSensor.getTempCByIndex(0);
}
float getTemperatureF() {
  tempSensor.requestTemperatures();
  return tempSensor.getTempFByIndex(0);
}

void displayLCD() {
  lcd.setCursor(0, 0); // Set the cursor to the first column and first row
  lcd.print("Temp C: ");
  lcd.print(getTemperatureC());
  lcd.setCursor(0, 1); // Set the cursor to the first column and 2nd row
  lcd.print("Temp F: ");
  lcd.print(getTemperatureF());
}