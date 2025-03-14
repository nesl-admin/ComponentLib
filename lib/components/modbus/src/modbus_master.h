#pragma once

#include <ModbusMaster.h>

//MODBUS slave devices
#define THERMOSTAT_1_ADDR 0x01

#define MODBUS_HOUSEKEEPING_INTERVAL 4000
#define MODBUS_UINT8_DEFAULT_VAL 127

//commands arriving via MQTT
#define MQTT_COMMAND_MB_OFF "off"
#define MQTT_COMMAND_MB_ON "on"

void thermostatLoop_A();
void thermostatLoop_B();
void setup_modbus_slaves();
void setup_modbus_master();
void loop_modbus_master();
void modbus_housekeeping();
void idle_waiting();

void modbus_mqtt_command(int id, uint16_t reg, uint16_t val);
void evse_reset_charge_session();

class Modbus_Looper : public ModbusMaster {
   public:
        Modbus_Looper() {};
        ~Modbus_Looper() {};

        void loop();
        void loop(void (*)());
        void (*_loop)();
};