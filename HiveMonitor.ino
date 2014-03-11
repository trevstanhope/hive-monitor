/* 
  HiveMonitor
  Developed by Trevor Stanhope
  DAQ controller for hive sensor monitoring.

  TODO:
  - SD logging
*/

/* --- Libraries --- */
#include "stdio.h"
#include <DHT.h>
#include <SoftwareSerial.h>
#include <SD.h>

/* --- Pins --- */
#define SD_PIN 10
#define DHT_INTERNAL_PIN A0
#define DHT_EXTERNAL_PIN A1
#define RPI_POWER_PIN A5

/* --- Values --- */
#define DHT_TYPE DHT11
#define BAUD 9600
#define CHARS 8
#define BUFFER 128
#define DIGITS 4
#define PRECISION 2
#define INTERVAL 1
#define UP_TIME 5
#define DOWN_TIME 10

/* --- Functions --- */
float get_int_C(void);
float get_int_RH(void);
float get_ext_C(void);
float get_ext_RH(void);

/* --- Objects --- */
DHT internal(DHT_INTERNAL_PIN, DHT_TYPE);
DHT external(DHT_EXTERNAL_PIN, DHT_TYPE);

/* --- Strings --- */
char int_C[CHARS];
char int_RH[CHARS];
char ext_C[CHARS];
char ext_RH[CHARS];
char json[BUFFER];
char csv[BUFFER];

/* --- State --- */
int rpi_timer = 0; // seconds on
boolean rpi_on = true; // start on

/* --- Setup --- */
void setup() {
  
  // Setup Serial
  Serial.begin(BAUD);
  
  // Setup Relay
  pinMode(RPI_POWER_PIN, OUTPUT);
  digitalWrite(RPI_POWER_PIN, LOW); // start on
  
  // Setup SD
  pinMode(SD_PIN, OUTPUT);
  if (!SD.begin(SD_PIN)) {
    return;
  }
  
  // Setup Sensors
  internal.begin();
  external.begin();
}

/* --- Loop --- */
void loop() {
  
  // Read Sensors
  dtostrf(get_ext_RH(), DIGITS, PRECISION, ext_RH); 
  dtostrf(get_ext_C(), DIGITS, PRECISION, ext_C);
  dtostrf(get_int_RH(), DIGITS, PRECISION, int_RH);
  dtostrf(get_int_C(), DIGITS, PRECISION, int_C);
  sprintf(json, "{'int_temp':%s, 'ext_temp':%s, 'int_humidity':%s, 'ext_humidity':%s}", int_C, ext_C, int_RH, ext_RH);
  sprintf(csv, "%s, %s, %s, %s", int_C, ext_C, int_RH, ext_RH);

  // Log to CSV-file
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(csv);
    dataFile.close();
  }
  
  // Send to RaspberryPi
  Serial.println(json);
  delay(1000*INTERVAL);
  Serial.flush();
  
  // Set RaspberryPi ON/OFF State
  rpi_timer += INTERVAL;
  if (rpi_on) {
    if (rpi_timer > UP_TIME) {
      rpi_timer = 0;
      rpi_on = false;
      digitalWrite(RPI_POWER_PIN, LOW);
    }
  }
  else {
    if (rpi_timer > DOWN_TIME) {
      rpi_timer = 0;
      rpi_on = true;
      digitalWrite(RPI_POWER_PIN, HIGH);
    }
  }
}

/* --- Get Internal Humidity --- */
float get_int_RH() {
  float val = internal.readHumidity();
  if (isnan(val)) {
    return 0;
  }
  else {
    return val;
  }
}

/* --- Get Internal Temperature --- */
float get_int_C() {
  float val = internal.readTemperature(); //  Serial.println(val);
  if (isnan(val)) {
    return 0;
  }
  else {
    return val;
  }
}

/* --- Get External Humidity --- */
float get_ext_RH() {
  float val = external.readHumidity();
  if (isnan(val)) {
    return 0;
  }
  else {
    return val;
  }
}

/* --- Get External Temperature --- */
float get_ext_C() {
  float val = external.readTemperature();
  if (isnan(val)) {
    return 0;
  }
  else {
    return val;
  }
}
