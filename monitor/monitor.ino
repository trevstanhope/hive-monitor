
/* 
  HiveMind Arduino Sensor Monitor
  Developed by Trevor Stanhope
  DAQ controller for single-hive urban beekeepers.
*/

/* --- Libraries --- */
#include "stdio.h"
#include <DHT.h>
#include <SoftwareSerial.h>

/* --- Pins --- */
#define DHT_INTERNAL_PIN A0
#define DHT_EXTERNAL_PIN A2

/* --- Values --- */
#define DHT_TYPE DHT22
#define BAUD 9600
#define CHARS 8
#define BUFFER 128
#define DIGITS 4
#define PRECISION 2
#define INTERVAL 1000

/* --- Functions --- */
float get_int_T(void);
float get_int_RH(void);
float get_ext_T(void);
float get_ext_RH(void);

/* --- Objects --- */
DHT internal(DHT_INTERNAL_PIN, DHT_TYPE);
DHT external(DHT_EXTERNAL_PIN, DHT_TYPE);
char int_T[CHARS];
char int_RH[CHARS];
char ext_T[CHARS];
char ext_RH[CHARS];
char output[BUFFER];
char int_T_key[] = "internal_temperature";
char ext_T_key[] = "external_temperature";
char int_RH_key[] = "internal_humidity";
char ext_RH_key[] = "external_humidity";

/* --- Setup --- */
void setup() {
  
  // Setup Serial
  Serial.begin(BAUD);
  
  // Setup Sensors
  internal.begin();
  external.begin();
  
}

/* --- Loop --- */
void loop() {
  
  // Read Sensors
  dtostrf(get_int_T(), DIGITS, PRECISION, int_T); 
  dtostrf(get_ext_T(), DIGITS, PRECISION, ext_T);
  dtostrf(get_int_RH(), DIGITS, PRECISION, int_RH);
  dtostrf(get_ext_T(), DIGITS, PRECISION, ext_RH);
  sprintf(output, "{'%s':%s, '%s':%s, '%s':%s, '%s':%s}", int_T_key, int_T, ext_T_key, ext_T, int_RH_key,int_RH, ext_RH_key, ext_RH);
  
  // Send to RaspberryPi
  Serial.println(output);
  delay(INTERVAL);
  Serial.flush();
 
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
float get_int_T() {
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
float get_ext_T() {
  float val = external.readTemperature();
  if (isnan(val)) {
    return 0;
  }
  else {
    return val;
  }
}
