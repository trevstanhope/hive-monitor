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
#define UP_TIME 5 // when it will turn back on
#define DOWN_TIME 10 // when it will turn off
#define WAIT_TIME 2 // time for RPi to close

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
char volts[CHARS];
char amps[CHARS];
char relay[CHARS];

/* --- Line Buffers --- */
char json[BUFFER];
char csv[BUFFER];

/* --- State --- */
int time = 0; // seconds on
int a = 0;
int b = 0;

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
  
  // Start time
  a = millis();
  
  // Read all sensors always
  dtostrf(get_ext_RH(), DIGITS, PRECISION, ext_RH); 
  dtostrf(get_ext_C(), DIGITS, PRECISION, ext_C);
  dtostrf(get_int_RH(), DIGITS, PRECISION, int_RH);
  dtostrf(get_int_C(), DIGITS, PRECISION, int_C);
  dtostrf(get_volts(), DIGITS, PRECISION, volts);
  dtostrf(get_amps(), DIGITS, PRECISION, amps);
  dtostrf(get_relay(), DIGITS, PRECISION, relay);

  // Always try to log everything to CSV-file
  sprintf(csv, "%s, %s, %s, %s", int_C, ext_C, int_RH, ext_RH);;
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(csv);
    dataFile.close();
  }
  
  // Always try to send over serial
  sprintf(json, "{'int_temp':%s, 'ext_temp':%s, 'int_humidity':%s, 'ext_humidity':%s, 'voltage':%s, 'amperage':%s, 'relay':%d}", int_C, ext_C, int_RH, ext_RH, volts, amps, relay);
  Serial.println(json);
  delay(1000 * INTERVAL);
  Serial.flush();
  
  // End time
  b = millis();
  time = time + (b-a);
}

/* --- Get Relay State --- */
// Keep on until UPTIME, then off until UPTIME+DOWNTIME, but WAIT wait until NODE is OFF
// -1 is OFF
// 0 is WAITING
// 1 is ON
int get_relay() {
  int val;
  if (time <= UP_TIME) {
    digitalWrite(RPI_POWER_PIN, LOW);
    val = 1;
  } else if (time <= UP_TIME + WAIT_TIME) {
    digitalWrite(RPI_POWER_PIN, LOW);
    val = 0;
  } else if (time <= UP_TIME + WAIT_TIME + DOWN_TIME) {
    digitalWrite(RPI_POWER_PIN, HIGH);
    val = -1;
  } else {
    val = 1;
    time = 0;
  }
  return val;
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

/* --- Get Amperage --- */
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

/* --- Get Voltage --- */
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
