#include <SoftwareSerial.h>
#include "gps.h"

#define POLL_INTERVAL 300000 //5 mins

//offset lat/lon to obfuscate actual position(set to 0 to go live)
#define FUDGE_LAT 3.1234
#define FUDGE_LON 12.4342

//RX, TX are MCU pins, not GPS (ie. MCU RX pin connects to "TX" on GPS; MCU TX to "RX" on GPS)
SoftwareSerial ss(GPS_SERIAL_RX, GPS_SERIAL_TX);
TinyGPSPlus gps;

TinyGPSPlus* get_gps() {
    return &gps;
}

void displayInfo() {
  Serial.print(F("Location: "));
  if (gps.location.isValid()) {
    Serial.print(gps.location.lat(), 6);
    if (gps.location.lat() > 0) {
    Serial.print(F(" N,"));
    } else {
      Serial.print(F(" S,"));
    }
    Serial.print(gps.location.lng(), 6);
    if (gps.location.lng() > 0) {
      Serial.print(F(" E, "));
    } else {
      Serial.print(F(" W, "));
    }
    //Serial.print(gps.location.rawLat().deg, 6);
    //Serial.print(F(","));
    //Serial.print(gps.location.rawLng().deg, 6);
    //Serial.print(F(","));
    Serial.print(gps.speed.kmph(), 0);
    Serial.print(F("kmph, "));
    Serial.print(gps.altitude.meters(), 0);
    Serial.print(F("m msl, #SAT:"));
    Serial.print(gps.satellites.value(), 6);

  } else {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid()) {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  } else {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid()) {
    if (gps.time.hour() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  } else {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}

void setup_gps() {
  //RX/tx ON MCU pins, NOT GPS pin labels (ie. our TX goes to GPS RX)
  Serial.printf("GPS init\n");
  pinMode(GPS_SERIAL_RX, INPUT);
  pinMode(GPS_SERIAL_TX, OUTPUT);
  ss.begin(9600);
}

int gps_interval = 0;
void loop_gps_ok() {
  while (ss.available() > 0) {
    //Serial.write(ss.read());
    gps.encode(ss.read());
  }
  if (millis() - gps_interval > POLL_INTERVAL) {
    if (gps.location.isValid()) {
      Serial.printf("GPS COORDS: lat,lon: %3.6f, %3.6f\n", gps.location.lat(), gps.location.lng());
    } else {
      Serial.println("GPS LOC INVALID");
    }
    gps_interval = millis();
  }
}

void loop_gps() {
  while (ss.available() > 0) {
    //Serial.write(ss.read());
    //gps.encode(ss.read());
    ss.flush();
  }
  if (millis() - gps_interval > POLL_INTERVAL) {
    int start_millis = millis();
    do {
      while (ss.available() > 0) {
        //Serial.write(ss.read());
        gps.encode(ss.read());
      }
    } while (millis() - start_millis < 1000);
    if (gps.location.isValid()) {
      Serial.printf("GPS COORDS: lat,lon: %3.6f, %3.6f\n", gps.location.lat(), gps.location.lng());
    } else {
      Serial.println("GPS LOC INVALID");
    }
    gps_interval = millis();
  }
}