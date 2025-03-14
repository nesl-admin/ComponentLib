#include <Arduino.h>
#include "button.h"
#include <modbus.h>
#include <gps.h>
#include <modem.h>
#include <display.h>

void setup() {
  setup_buttons();
  setup_gps();
  setup_modbus_master();
  setup_modem();
  setup_display();
}

void loop() {
  loop_buttons();
  loop_gps();
  loop_modbus_master();
  loop_modem();
  loop_display();
}