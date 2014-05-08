/* 
  HiveMonitor
  Developed by Trevor Stanhope
  DAQ controller for hive sensor monitoring.
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
#define DHT_TYPE DHT22
#define BAUD 9600
#define CHARS 8
#define BUFFER 128
#define DIGITS 4
#define PRECISION 2
#define ON_INTERVAL 500
#define OFF_INTERVAL 100
#define BOOT_WAIT 60000
#define TIMEOUT 20
#define UP_TIME 30 // seconds until when it will turn off
#define DOWN_TIME 60 // seconds until when it will back turn on
#define PIN_WAIT 200

/* --- Functions --- */
float get_int_temp(void);
float get_int_humidity(void);
float get_ext_temp(void);
float get_ext_humidity(void);
float get_volts(void);
float get_amps(void);

/* --- Objects --- */
DHT INT_DHT(DHT_INTERNAL_PIN, DHT_TYPE);
DHT EXT_DHT(DHT_EXTERNAL_PIN, DHT_TYPE);

/* --- Strings --- */
char INT_T[CHARS];
char INT_H[CHARS];
char EXT_T[CHARS];
char EXT_H[CHARS];
char VOLTS[CHARS];
char AMPS[CHARS];

/* --- Line Buffers --- */
char JSON[BUFFER];
char CSV[BUFFER];
char COMMAND;

/* --- State --- */
int TIME = 0; // seconds on

/* --- Setup --- */
void setup() {
  digitalWrite(RPI_POWER_PIN, HIGH); // Start with relay on
  pinMode(RPI_POWER_PIN, OUTPUT);
  delay(BOOT_WAIT); // Serial cannot be on during RPi boot
  Serial.begin(BAUD);
  Serial.setTimeout(TIMEOUT);
  pinMode(SD_PIN, OUTPUT);
  if (!SD.begin(SD_PIN)) {
    return;
  }
  INT_DHT.begin();
  EXT_DHT.begin();
}

/* --- Loop --- */
void loop() {
  TIME++;
  dtostrf(get_ext_temp(), DIGITS, PRECISION, EXT_T); 
  dtostrf(get_ext_humidity(), DIGITS, PRECISION, EXT_H);
  dtostrf(get_int_temp(), DIGITS, PRECISION, INT_T);
  dtostrf(get_int_humidity(), DIGITS, PRECISION, INT_H);
  dtostrf(get_volts(), DIGITS, PRECISION, VOLTS);
  dtostrf(get_amps(), DIGITS, PRECISION, AMPS);
  sprintf(CSV, "%d,%s,%s,%s,%s,%s,%s", TIME, INT_T, EXT_T, INT_H, EXT_H, VOLTS, AMPS);
  File datafile = SD.open("datalog.txt", FILE_WRITE);
  if (datafile) {
    datafile.println(CSV);
    datafile.close();
  }
  if (TIME <= UP_TIME) {
    sprintf(JSON, "{'cycles':%d,'int_t':%s,'ext_t':%s,'int_h':%s,'ext_h':%s,'volts':%s,'amps':%s}", UP_TIME - TIME, INT_T, EXT_T, INT_H, EXT_H, VOLTS, AMPS);
    Serial.println(JSON);
    delay(ON_INTERVAL);
  }
  else if (TIME <= DOWN_TIME) {
    digitalWrite(RPI_POWER_PIN, LOW);
    Serial.end();
    delay(OFF_INTERVAL);
  }
  else {
    TIME = 0;
    digitalWrite(RPI_POWER_PIN, HIGH);
    delay(BOOT_WAIT);
    Serial.begin(BAUD);
  }
}

/* --- Sensor Functions --- */
// Internal Humidity
float get_int_humidity() {
  float val = INT_DHT.readHumidity();
  if (isnan(val)) {
    return 0;
  }
  else {
    return val;
  }
}

// Internal Temperature
float get_int_temp() {
  float val = INT_DHT.readTemperature(); //  Serial.println(val);
  if (isnan(val)) {
    return 0;
  }
  else {
    return val;
  }
}

// Get External Humidity
float get_ext_humidity() {
  float val = EXT_DHT.readHumidity();
  if (isnan(val)) {
    return 0;
  }
  else {
    return val;
  }
}

// Get External Temperature
float get_ext_temp() {
  float val = EXT_DHT.readTemperature();
  if (isnan(val)) {
    return 0;
  }
  else {
    return val;
  }
}

// Amperage
float get_amps() {
//  float val = external.readTemperature();
//  if (isnan(val)) {
//    return 0;
//  }
//  else {
//    return 1;
//  }
  return 1;
}

// Voltage
float get_volts() {
//  float val = external.readTemperature();
//  if (isnan(val)) {
//    return 0;
//  }
//  else {
//    return 1;
//  }
  return 1;
}
