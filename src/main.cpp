#include <Arduino.h>
#include "button.h"
#include <modbus.h>
#include <gps.h>

// put function declarations here:
int myFunction(int, int);

void setup() {
  setup_buttons();
  setup_gps();
  setup_modbus_master();
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}