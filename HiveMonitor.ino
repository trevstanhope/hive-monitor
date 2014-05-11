/* 
  HiveMonitor
  Developed by Trevor Stanhope
  DAQ controller for hive sensor monitoring.
  
  Possible problems with serial:
    - serial in gets clogged with garbage
    - sometimes it takes a while to establish the serial connection to the rpi
    - sometimes it takes a while to connect to the wifi, increasing the boot wait time
*/

/* --- Libraries --- */
#include "stdio.h"
#include <DHT.h>

/* --- Definitions --- */
#define DHT_INTERNAL_PIN A0
#define DHT_EXTERNAL_PIN A1
#define DHT_TYPE DHT22
#define RESET_PIN A4
#define RPI_POWER_PIN A5

/* --- Constants --- */
const int BAUD = 9600;
const int CHARS = 8;
const int BUFFER = 128;
const int DIGITS = 4;
const int PRECISION = 2;
const int ON_WAIT = 500;
const int OFF_WAIT = 1000;
const int BOOT_WAIT = 60000;
const int RESET_WAIT = 500; //
const int PIN_WAIT = 200; // wait for pin to initialize
const int SERIAL_WAIT = 1000; // wait for serial connection to start
const int SHUTDOWN_WAIT = 5000; // wait for pi to shutdown
const int ON_CYCLES = 60; // counter value when it will turn off
const int OFF_CYCLES = 1200; // counter value when it will back turn on

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

/* --- Variables --- */
char INT_T[CHARS];
char INT_H[CHARS];
char EXT_T[CHARS];
char EXT_H[CHARS];
char VOLTS[CHARS];
char AMPS[CHARS];
char JSON[BUFFER];
int CYCLES = 0;
int INCOMING = 0;

/* --- Setup --- */
void setup() {
  digitalWrite(RESET_PIN, HIGH);
  digitalWrite(RPI_POWER_PIN, HIGH);
  delay(PIN_WAIT);
  pinMode(RESET_PIN, OUTPUT);
  pinMode(RPI_POWER_PIN, OUTPUT);
  delay(BOOT_WAIT); // Serial cannot be on during RPi boot
  Serial.begin(BAUD);
  delay(SERIAL_WAIT); // wait for serial to establish
  INT_DHT.begin();
  EXT_DHT.begin();
}

/* --- Loop --- */
void loop() {
  // Flush incoming serial buffer to prevent mem leaks
  while (Serial.available() > 0) {
    INCOMING = Serial.read();
  }
  if (CYCLES < ON_CYCLES) {
    dtostrf(get_ext_temp(), DIGITS, PRECISION, EXT_T); 
    dtostrf(get_ext_humidity(), DIGITS, PRECISION, EXT_H);
    dtostrf(get_int_temp(), DIGITS, PRECISION, INT_T);
    dtostrf(get_int_humidity(), DIGITS, PRECISION, INT_H);
    dtostrf(get_volts(), DIGITS, PRECISION, VOLTS);
    dtostrf(get_amps(), DIGITS, PRECISION, AMPS);
    sprintf(JSON, "{'cycles':%d,'int_t':%s,'ext_t':%s,'int_h':%s,'ext_h':%s,'volts':%s,'amps':%s}", CYCLES, INT_T, EXT_T, INT_H, EXT_H, VOLTS, AMPS);
    Serial.println(JSON);
    delay(ON_WAIT);
  }
  else if (CYCLES == ON_CYCLES) {
    Serial.flush();
    Serial.end();
    delay(SHUTDOWN_WAIT);
    digitalWrite(RPI_POWER_PIN, LOW);
  }
  else if (CYCLES < (ON_CYCLES + OFF_CYCLES)) {
    delay(OFF_WAIT);
  }
  else {
    digitalWrite(RESET_PIN, LOW);
  }
  CYCLES++;
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
