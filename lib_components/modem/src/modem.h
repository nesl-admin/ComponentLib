#pragma once
#include <Arduino.h>
#include <TinyGSM.h>

#define SIZE_MSG_BUFFER 4096

#define AT_CMD_DELETE_MESSAGE_D         "AT+CMGD=%d\r\n" //TODO: sendAT wrapper to use printf formatting
#define AT_CMD_DELETE_MESSAGE_S         "AT+CMGD=%s\r\n" //TODO: sendAT wrapper to use printf formatting
#define AT_CMD_DISABLE_RF               "+CFUN=0"
#define AT_CMD_ENABLE_RF                "+CFUN=1"
#define AT_CMD_CONFIGURE_TEXT_MODE      "+CMGF=1"
#define AT_CMD_MESSAGE_READ_D           "AT+CMGR=%d\r\n"
#define AT_CMD_MESSAGE_READ_S           "AT+CMGR=%s\r\n"

#if defined(TINY_GSM_MODEM_SIM7080) || defined(TINY_GSM_MODEM_SIM7070)
#define SIM7xxxRegStatus SIM70xxRegStatus
#else
#define SIM7xxxRegStatus SIM7600RegStatus
#endif

void setup_modem();
void loop_modem();
void get_time(bool print);
//void send_wakeup_msg();
void state_machine(void* pvParameters);

typedef enum {
    PROBE_AT_SERIAL,
    STARTUP_RESET,
    WAITING_FOR_NETWORK,
    CHECK_NETWORK,
    ONLINE,
    READING_STORED_MESSAGES
} ModemState;

class ModemShim : public TinyGsm {
    public:
    ModemShim();
    ~ModemShim() {};
};

class Modem : public ModemShim {
    public:
        Modem();
        ~Modem() {};

   // TinyGsm sim;
    ModemState current_state;
    ModemState state() { return current_state;}
    bool isOnline() { return (current_state == ONLINE); }

    void setup();
    void print_modem_info();
    void get_time(bool print);


};

extern Modem theModem;
extern HardwareSerial ModemSerial;
extern const char* register_info[];
