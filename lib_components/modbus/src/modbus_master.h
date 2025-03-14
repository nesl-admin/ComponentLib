#pragma once

#include <ModbusMaster.h>
//#include <pins.h>

/*  TODO per EVPE  stats per phase
    Number of EVSE active charging
    Total kW of all active EVSE
    Total kWh of all EVSE in schedule 
    Total kWh of all EVSE in day - assume 10 pm schdule change of day
    Peak kW of all active EVSE in schedule
    Peak kWh of all EVSE in schedule 
    Peak kWh of all EVSE in day - assume 10 pm schdule change of day
    Number of EVSE Phases active  
    Number of EVSE provisioned 
    Number of EVSE inservice 
    MQTT reports stats  
    Last MQTT report time
    Uptime 
    Last command Rx time
    Last DTM Power Report Rx time 
    Max power used in schedule 
    Max power used in day
    Total daily Power Report (10PM-6AM 
    Future – WiFi/BLE names and last reported
    Future - next OTA checkin - yr month day hour 
    Future - last OTA update - yr month day hour 
*/

/*
Watch out for MAX485 DO VS D1 unloaded SERIAL SIDE AFFECTS - SEE TECH DETAILS AT  https://www.analog.com/en/products/max485.html#part-details
BIAS RESISTORS DETAILED TECH SPECS HERE https://control.com/forums/threads/modbus-standard-termination.20389/
***** must use 620-150 OHM TERMINATION RESISTOR AT A-B FAR END TERMINATIO TO MINIMIZE REFLECTIONS ******

TESTING WITH 150 OHM RESISTOR ACROSS LAST FURTHEST A-B MODBUS RTU ENDPOINT THIS CAN BE 620 OHM ALTERNATIVELY - FURTHER SCOPE TESTING REQUIRED ON CAT5E VS STP RS485 CBLE 
*/

//MODBUS slave devices
#define THERMOSTAT_1_ADDR 0x01 // TODO For MVP POC a 3rd party modbus temp/humidity/pressure sensors is queried - ideally door contact gpio

#define MODBUS_HOUSEKEEPING_INTERVAL 4000
#define MODBUS_UINT8_DEFAULT_VAL 127

//commands arriving via MQTT
#define MQTT_COMMAND_MB_OFF "off"
#define MQTT_COMMAND_MB_ON "on"

//These are EVPE totals used by EVSE events and alarms policy enforcment rules and debugging 
//TODO could make these arrays of EVSEid, EVSEid is unique only to EVPE (modbus node number - in future Homme Plug EVSE )
// EVSE {ready, connected, active_charging, active_discharging, now_charging,now_discharging, discharged, charged, EVSE_fault stopped} 
extern int EVSE_Active_Charging;
extern int EVSE_Active_Charging_PhaseA;     // number of active EVSE phaseA or phase1
extern int EVSE_Active_Charging_PhaseB;     // number of active EVSE phaseB or phase2
extern int EVSE_Active_Charging_PhaseC;     // number of active EVSE phaseC or phase3
extern int EVSE_Num_Phases_Connected;       // number of phases with EV cars connected to EVSE
extern int EVSE_Num_Phases_Active;          // number of phases with EV cars active charging to EVSE
extern int EVSE_In_Service;                 // number of subtending EVSE in servicea stopped EVSE is taken out of service count
extern int EVSE_Stopped;                    // number of subtending EVSE staged or configured or stopped
extern int EVSE_Configured; 
extern int EVSE_faulted;                    // number of EVSE that have internal fault condition
extern int EVSE_car_faulted;                // number of subtending EVSE reporting car faulted - charger can signal its got a fault
extern int EVSE_Last_Leakage_reported;       // last time RCMU at EVSE reported 6 ma DC or 30 ma AC leakage
extern int EVSE_Last_MQTT_Report;           // last time a MQTT event was reported - TODO should become eventid and timestamp
extern int EVSE_Uptime;                     // time its been inservice ready or beyond
extern int EVSE_Max_Power_Reported;
extern int EVSE_Total_Daily_Power_Reported;

void thermostatLoop_A();
void thermostatLoop_B();
void setup_modbus_slaves();
void setup_modbus_master();
void loop_modbus_master();
void modbus_housekeeping();   //TODO should do stats here on modbus faults 
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