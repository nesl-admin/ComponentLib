#include <Arduino.h>
#include <components.h>

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
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