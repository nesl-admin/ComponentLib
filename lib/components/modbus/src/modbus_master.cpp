#include <SoftwareSerial.h>
#include <modbus.h>
#include <pins_modbus.h>

#define SHT20_MODBUS_ADDR 1 //TODO move to config
#define POLL_INTERVAL 300000 //5 mins

SoftwareSerial _modbus1(RS485_RX_1, RS485_TX_1); //(rx, tx) corresponds with HW519 rxd txd pins
SoftwareSerial _modbus2(RS485_RX_2, RS485_TX_2); //(rx, tx) corresponds with HW519 rxd txd pins

//the temp/humidity sensor (SHT20)
Modbus_SHT20 sht20;


unsigned long lastMillis, lastEVSEMillis, lastEVSEChargingMillis = 0;
bool b_has_SHT20 = false;

void idle_waiting() {
  vTaskDelay(1);
}

void setup_sht20() {
  int mba = 1;
  b_has_SHT20 = true;
  Serial.printf("SETUP: MODBUS: SHT20 #1: address:%d\n",mba); 
  sht20.set_modbus_address(mba);
  sht20.begin(mba, _modbus1);
}

void setup_modbus_slaves() {
  //setup_thermostats();
  //setup_dtm();
  setup_sht20();
  //setup_evse();
}

void setup_modbus_master() {
  Serial.printf("MODBUS init\n");

  gpio_reset_pin(RS485_RX_1);
  gpio_reset_pin(RS485_TX_1);
  gpio_reset_pin(RS485_RX_2);
  gpio_reset_pin(RS485_TX_2);

  _modbus1.begin(9600);
  _modbus2.begin(9600);

  setup_modbus_slaves();

  sht20.poll();
}

void Modbus_Looper::loop(void (*loop)()) {
  _loop = loop;
}

void Modbus_Looper::loop() {
  if (_loop) {
    _loop();
  }
}

void loop_modbus_master() {
  if (millis() - lastMillis > POLL_INTERVAL) {
    sht20.poll();
    lastMillis = millis();
  }
}