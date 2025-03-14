#include <pins_modem.h>
#include <modem.h>


#ifdef LILYGO_T_SIM7080G
#define XPOWERS_CHIP_AXP2101
#include "XPowersLib.h"
#include "lilygo_utilities.h"
#define TINY_GSM_RX_BUFFER 1024
XPowersPMU  PMU;
bool  pmu_flag = false;
void setFlag(void) {
    pmu_flag = true;
}
#endif

enum {
    MODEM_CATM = 1,
    MODEM_NB_IOT,
    MODEM_CATM_NBIOT,
};

TaskHandle_t state_machine_task;
HardwareSerial ModemSerial(0);
Modem theModem;

const char gprsUser[] = "";
const char gprsPass[] = "";
const char* apn = "wholesale"; //Tello?
const char *register_info[] = {
    "Not registered, MT is not currently searching an operator to register to.The GPRS service is disabled, the UE is allowed to attach for GPRS if requested by the user.",
    "Registered, home network.",
    "Not registered, but MT is currently trying to attach or searching an operator to register to. The GPRS service is enabled, but an allowable PLMN is currently not available. The UE will start a GPRS attach as soon as an allowable PLMN is available.",
    "Registration denied, The GPRS service is disabled, the UE is not allowed to attach for GPRS if it is requested by the user.",
    "Unknown.",
    "Registered, roaming.",
};

int last_connection_check = 0;
SIM7xxxRegStatus status;

enum {
    Modem_CATM = 1,
    Modem_NB_IOT,
    Modem_CATM_NBIOT,
};
ModemShim::ModemShim() : TinyGsm(ModemSerial) {

}

Modem::Modem() {
    current_state = PROBE_AT_SERIAL;
}

void modem_power(bool turn_on) {
    if (turn_on) {
#ifdef TINY_GSM_MODEM_SIM7070
        digitalWrite(PIN_PWR_K, HIGH);
        delay(100);
        digitalWrite(PIN_PWR_K, LOW);
#elif defined(LILYGO_T_SIM7080G)
        // Pull down PWRKEY for more than 1 second according to manual requirements
        digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
        delay(1000);
        digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
        delay(1000);
        digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
        delay(100);
#endif
    }
}

void setup_power() {
    Serial.println("Initializing power.....");

#ifdef LILYGO_T_SIM7080G

    if (!PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
        Serial.println("Failed to initialize power.....");
        while (1) {
            delay(5000);
        }
    }

    // If it is a power cycle, turn off the ModemSerial power. Then restart it
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED ) {
        PMU.disableDC3();
        // Wait a minute
        delay(200);
    }

    //Set the working voltage of the ModemSerial, please do not modify the parameters
    PMU.setDC3Voltage(3000);    //SIM7080 ModemSerial main power channel 2700~ 3400V
    PMU.enableDC3();

    //ModemSerial GPS Power channel
    PMU.setBLDO2Voltage(3300);
    PMU.enableBLDO2();      //The antenna power must be turned on to use the GPS function

    // TS Pin detection must be disable, otherwise it cannot be charged
    PMU.disableTSPinMeasure();
    // Set the minimum common working voltage of the PMU VBUS input,
    // below this value will turn off the PMU
    PMU.setVbusVoltageLimit(XPOWERS_AXP2101_VBUS_VOL_LIM_4V36);

    // Set the maximum current of the PMU VBUS input,
    // higher than this value will turn off the PMU
    PMU.setVbusCurrentLimit(XPOWERS_AXP2101_VBUS_CUR_LIM_1500MA);

    // Set VSY off voltage as 2600mV , Adjustment range 2600mV ~ 3300mV
    PMU.setSysPowerDownVoltage(2600);

    /*********************************
     * step 2 : Enable internal ADC detection
    ***********************************/
    PMU.enableBattDetection();
    PMU.enableVbusVoltageMeasure();
    PMU.enableBattVoltageMeasure();
    PMU.enableSystemVoltageMeasure();

    /*********************************
     * step 3 : Set PMU interrupt
    ***********************************/
    pinMode(PMU_INPUT_PIN, INPUT);
    attachInterrupt(PMU_INPUT_PIN, setFlag, FALLING);

    // Disable all interrupts
    PMU.disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
    // Clear all interrupt flags
    PMU.clearIrqStatus();
    // Enable the required interrupt function
    PMU.enableIRQ(
        XPOWERS_AXP2101_BAT_INSERT_IRQ    | XPOWERS_AXP2101_BAT_REMOVE_IRQ      |   //BATTERY
        XPOWERS_AXP2101_VBUS_INSERT_IRQ   | XPOWERS_AXP2101_VBUS_REMOVE_IRQ     |   //VBUS
        XPOWERS_AXP2101_PKEY_SHORT_IRQ    | XPOWERS_AXP2101_PKEY_LONG_IRQ       |   //POWER KEY
        XPOWERS_AXP2101_BAT_CHG_DONE_IRQ  | XPOWERS_AXP2101_BAT_CHG_START_IRQ       //CHARGE
    );
    Serial.println("DCDC=======================================================================");
    Serial.printf("DC1  : %s   Voltage:%u mV \n",  PMU.isEnableDC1()  ? "+" : "-", PMU.getDC1Voltage());
    Serial.printf("DC2  : %s   Voltage:%u mV \n",  PMU.isEnableDC2()  ? "+" : "-", PMU.getDC2Voltage());
    Serial.printf("DC3  : %s   Voltage:%u mV \n",  PMU.isEnableDC3()  ? "+" : "-", PMU.getDC3Voltage());
    Serial.printf("DC4  : %s   Voltage:%u mV \n",  PMU.isEnableDC4()  ? "+" : "-", PMU.getDC4Voltage());
    Serial.printf("DC5  : %s   Voltage:%u mV \n",  PMU.isEnableDC5()  ? "+" : "-", PMU.getDC5Voltage());
    Serial.println("ALDO=======================================================================");
    Serial.printf("ALDO1: %s   Voltage:%u mV\n",  PMU.isEnableALDO1()  ? "+" : "-", PMU.getALDO1Voltage());
    Serial.printf("ALDO2: %s   Voltage:%u mV\n",  PMU.isEnableALDO2()  ? "+" : "-", PMU.getALDO2Voltage());
    Serial.printf("ALDO3: %s   Voltage:%u mV\n",  PMU.isEnableALDO3()  ? "+" : "-", PMU.getALDO3Voltage());
    Serial.printf("ALDO4: %s   Voltage:%u mV\n",  PMU.isEnableALDO4()  ? "+" : "-", PMU.getALDO4Voltage());
    Serial.println("BLDO=======================================================================");
    Serial.printf("BLDO1: %s   Voltage:%u mV\n",  PMU.isEnableBLDO1()  ? "+" : "-", PMU.getBLDO1Voltage());
    Serial.printf("BLDO2: %s   Voltage:%u mV\n",  PMU.isEnableBLDO2()  ? "+" : "-", PMU.getBLDO2Voltage());
    Serial.println("CPUSLDO====================================================================");
    Serial.printf("CPUSLDO: %s Voltage:%u mV\n",  PMU.isEnableCPUSLDO() ? "+" : "-", PMU.getCPUSLDOVoltage());
    Serial.println("DLDO=======================================================================");
    Serial.printf("DLDO1: %s   Voltage:%u mV\n",  PMU.isEnableDLDO1()  ? "+" : "-", PMU.getDLDO1Voltage());
    Serial.printf("DLDO2: %s   Voltage:%u mV\n",  PMU.isEnableDLDO2()  ? "+" : "-", PMU.getDLDO2Voltage());
    Serial.println("===========================================================================");

        /*
    The default setting is CHGLED is automatically controlled by the PMU.
    - XPOWERS_CHG_LED_OFF,
    - XPOWERS_CHG_LED_BLINK_1HZ,
    - XPOWERS_CHG_LED_BLINK_4HZ,
    - XPOWERS_CHG_LED_ON,
    - XPOWERS_CHG_LED_CTRL_CHG,
    * */
    PMU.setChargingLedMode(XPOWERS_CHG_LED_ON);

    // Print default setting current
    //printChargerConstantCurr();
#endif

     modem_power(true);
}

void Modem::print_modem_info() {
    bool res = isGprsConnected();
    Serial.print("GPRS status:");
    Serial.println(res ? "connected" : "not connected");

    String ccid = getSimCCID();
    Serial.print("CCID:");
    Serial.println(ccid);

    String imei = getIMEI();
    Serial.print("IMEI:");
    Serial.println(imei);

    String imsi = getIMSI();
    Serial.print("IMSI:");
    Serial.println(imsi);

    String cop = getOperator();
    Serial.print("Operator:");
    Serial.println(cop);

    IPAddress local = localIP();
    Serial.print("Local IP:");
    Serial.println(local);

    int csq = getSignalQuality();
    Serial.print("Signal quality:");
    Serial.println(csq);
}

void Modem::get_time(bool print) {
    int   ntp_year     = 0;
    int   ntp_month    = 0;
    int   ntp_day      = 0;
    int   ntp_hour     = 0;
    int   ntp_min      = 0;
    int   ntp_sec      = 0;
    float ntp_timezone = 0;
    bool ok = theModem.getNetworkTime(&ntp_year, &ntp_month, &ntp_day, &ntp_hour,
                                &ntp_min, &ntp_sec, &ntp_timezone);
    if (ok && print) {
        Serial.printf("Year: %d Month: %d Day: %d\n", ntp_year, ntp_month, ntp_day);
        Serial.printf("Hour: %d Minute: %d Second: %d\n", ntp_hour, ntp_min, ntp_sec);
        Serial.printf("Timezone: %3.2f\n", ntp_timezone);
         String time = theModem.getGSMDateTime(DATE_FULL);
        Serial.printf("Current Network Time: %s\n", time.c_str());
    }
}

void setup_online_modem() {
// Activate network bearer, APN can not be configured by default,
// if the SIM card is locked, please configure the correct APN and user password, use the gprsConnect() method
//#ifndef TINY_GSM_MODEM_SIM7070
    Serial.println("Activate network bearer");
    theModem.sendAT("+CNACT=0,1");
    int r = theModem.waitResponse();
    Serial.printf("Activate network bearer: %d\n",r);
    delay(1000);

    //Serial.println("GPRS connect");
    // if (!theModem.gprsConnect(apn, gprsUser, gprsPass)) {
    //    Serial.println("GPRS connect failed!");
    //     delay(1000);
        //break;
    //}

    bool res = theModem.isGprsConnected();
    Serial.print("GPRS status:");
    Serial.println(res ? "connected" : "not connected");
//#endif
}

void Modem::setup() {
    //rx, tx
    ModemSerial.begin(115200, SERIAL_8N1, PIN_MODEM_RX, PIN_MODEM_TX);
}

void apn_setup() {
    //Set the APN manually. Some operators need to set APN first when registering the network.
    //  Serial.println("Set APN 1");
    //  theModem.sendAT("+CGDCONT=1,\"IP\",\"", apn, "\"");
    //  r = theModem.waitResponse();
    //  Serial.printf("Set operators apn: %d\n",r);
    //  delay(1000);
    
    //Serial.println("Set APN 2");
    //theModem.sendAT("+CNCFG=0,1,\"", apn, "\"");
    //if (theModem.waitResponse() != 1) {
    //    Serial.println("Config apn Failed!");
    //    delay(1000);
        //   break;
    //}
}

int signal_strength_bars(int rssi) {
    int bars = 0;
    if (rssi >= 30) { 
        bars = 5;
    } else if (rssi < 30 & rssi > 26) {
        bars = 4;
    } else if (rssi < 25 & rssi > 20) {
        bars = 3;
    } else if (rssi < 20 & rssi > 15) {
        bars = 2;
    } else if (rssi < 15 & rssi > 10) {
        bars = 1;
    }
    return bars;
}

void log(const char* report) {

}

int waiting_ticks = 0;
bool first_pass_setup = true;
int bad_reg_count = 0;
void state_machine(void* pvParameters) {
    int not_responding_retries = 0;
    while(1) {
       // if (xSemaphoreTake(modem_mutex, 0)){ //100)) {
            switch (theModem.current_state) {
                case PROBE_AT_SERIAL:
                    Serial.println("modem: start: probe AT serial");
                        log("start");
                        if (theModem.testAT(100)) {
                            not_responding_retries = 0;
                            Serial.println("modem: started");
                            theModem.current_state = STARTUP_RESET;
                            break;
                        }
                        if (not_responding_retries++ > 30) {
                            not_responding_retries = 0;
                            Serial.println("modem: serial not responding - reset");
                            ModemSerial.flush();
                            //ensure modem is powered up
                            modem_power(true);
                        }
                    break;

                case STARTUP_RESET: {
                    //TODO refactor into func
                    log("reset");

                    if (!theModem.testAT(500)) {
                        if (!theModem.testAT(500)) {
                           theModem.current_state = PROBE_AT_SERIAL;
                           break;
                        }
                    }                
                    Serial.println("modem: startup/reset");
                    /*********************************
                     * Check if the SIM card is inserted
                    ***********************************/
                    Serial.println("Get SIM status");
                    if (theModem.getSimStatus() != SIM_READY) {
                        Serial.println("No SIM Card - probe serial and reset");
                        theModem.current_state = PROBE_AT_SERIAL;
                        break;
                    }
                    ModemSerial.flush();
                    int r = 0;
#ifdef false
                    // Disable RF
                    Serial.println("Disable RF");
                    theModem.sendAT("+CFUN=0");
                    r = theModem.waitResponse(20000UL);
                    Serial.printf("Disable RF: %d\n",r);
                    delay(1000);
                                      // Enable RF
                                      Serial.println("Enable RF");
                                      theModem.sendAT("+CFUN=1");
                                      r = theModem.waitResponse(12000);
                                      Serial.printf("Enable RF: %d\n",r);
                                      delay(0);
#endif
                    Serial.println("Configure TEXT mode");
                    theModem.sendAT("+CMGF=1"); // Configure TEXT mode
                    r = theModem.waitResponse(2000);
                    Serial.printf(" Set text mode: %d\n",r);
                    delay(1000);
    
                    // Disable RF
                   // Serial.println("Disable RF");
                    //theModem.sendAT("+CFUN=0");
                    //r = theModem.waitResponse(20000UL);
                    //Serial.printf("Disable RF: %d\n",r);
                        
                    /*********************************
                    * Set the network mode
                    ***********************************/
                    theModem.setPreferredMode(MODEM_CATM);///+CMNB=1
#ifdef TINY_GSM_MODEM_SIM7070
                    theModem.setNetworkMode(38);    //use LTE  +CNMP=38
#else
                    theModem.setNetworkMode(2);    //use automatic +CNMP=2
#endif
                    //uint8_t pre = theModem.getPreferredMode(); //CMNB?
                    //uint8_t mode = theModem.getNetworkMode(); //CMNP?
                    //Serial.printf("getNetworkMode:%u, getPreferredMode:%u\n", mode, pre);
    
                    apn_setup();

                    // Enable RF
                    Serial.println("Enable RF");
                    theModem.sendAT("+CFUN=1");
                    if (theModem.waitResponse(20000UL) != 1) {
                        Serial.println("Enable RF Failed!");
                        delay(1000);
                        break;
                    }
    
                    
                    //cmd echo on/off
                    //ModemSerial.println("ATE1");
    
                    //store new messages in SM memory
                    Serial.println("Message storage setup...");
    #ifdef LILYGO_T_SIM7080G
                    theModem.sendAT("+CPMS=\"SM\""); //SIM message storage
                    //theModem.sendAT("+CPMS=\"ME\",\"ME\",\"ME\""); //Mobile Equipment
    #else
                    theModem.sendAT("+CPMS=\"SM\",\"SM\",\"SM\""); //SIM message storage
                    //theModem.sendAT("+CPMS=\"ME\",\"ME\",\"ME\""); //SIM message storage
    #endif
                    r = theModem.waitResponse(5000);
                    Serial.printf("SMS message storage setup: %d\n",r);
                    delay(1000);

                    //local time zone
                    theModem.sendAT("+CLTS=1");
                    r = theModem.waitResponse(200);
                    Serial.printf("local Time zone setting: %d\n",r);

                    //<mt> = mobile terminated messages
                    //<bm> = broadcast messages
                    //<ds> = status report (?)
                    //<bfr> = buffer
    
                    //<mode>: 2: 
                    //  Buffer unsolicited result codes in the TA when TA-TE link is reserved (e.g. in on-line data mode) and 
                    //  flush them to the TE after reservation. Otherwise forward them directly to the TE
                    //
                    //<mt>: 2:
                    //  SMS-DELIVERs (except class 2) are routed directly to the TE using unsolicited result code:
                    //  +CMT: [<alpha>],<length><CR><LF><pdu> (PDU mode enabled)or
                    //  +CMT: <oa>,[<alpha>],<scts>[,<tooa>,<fo>,<pid>,<dcs>,<sca>,<tosca>,<length>]<CR><LF><data>
                    //<bm>: 0: No CBM indications are routed to the TE.
                    //<ds>: 0: No SMS-STATUS-REPORTs are routed to the TE.
                    //<bfr>: 0: TA buffer of unsolicited result codes defined within this Command
                    //   is flushed to the TE when <mode> 1...3 is entered (OK response shall be given before flushing the codes)
    
                    // Decides how newly arrived SMS messages should be handled
                    //CNMI setting: '2,2,0,0,0': delivered immediately and not stored
                    //CNMI setting: '2,1,0,0,0': stored and TE notified with +CMTI and msg index
                    Serial.println("New message setup...");
                    theModem.sendAT("+CNMI=2,1,0,0,0");
                    r = theModem.waitResponse(6000);
                    Serial.printf("SMS New message setup: %d\n",r);
    
                    // Enable RF
                    //Serial.println("Enable RF");
                    //theModem.sendAT("+CFUN=1");
                   // r = theModem.waitResponse(12000);
                   // Serial.printf("Enable RF: %d\n",r);
                   // delay(0);
                    Serial.println("Waiting for network...");
                    theModem.current_state = WAITING_FOR_NETWORK;
                
                    break;
                }

                case WAITING_FOR_NETWORK:
                case CHECK_NETWORK:

                    status = theModem.getRegistrationStatus();

                    if (status == REG_SEARCHING) {
                        char buf[13];
                        sprintf(buf,"search:%02d\0",theModem.getSignalQuality());
                        //display_set_rssi(buf);
                        Serial.print(".");
                        if (waiting_ticks++ > 30) {
                            Serial.printf("\nsignal quality:%d\n", theModem.getSignalQuality());
                            waiting_ticks = 0;
                        }
                    } else if (status == REG_UNKNOWN || status == REG_NO_RESULT || status == REG_UNREGISTERED) {
                        //display_set_rssi("???");
                        Serial.print("?");
                        if (waiting_ticks++ > 30) {
                            int sig = theModem.getSignalQuality();
                            Serial.printf("\nsignal quality:%d\n", sig);
                            waiting_ticks = 0;
                        }
                        if (bad_reg_count++ > 120 && status == REG_UNREGISTERED && theModem.getSignalQuality() == 99) {
                            bad_reg_count = 0;
                            //clear possible disallowed network
                            theModem.sendAT("+CRSM=214,28539,0,0,12,\"FFFFFFFFFFFFFFFFFFFFFFFF\"");
                            delay(1000);
                            setup_online_modem();
                            theModem.current_state = STARTUP_RESET;
                        }
                    } else if (status == REG_DENIED) {
                        Serial.printf("\n**** REGISTRATION DENIED ****\n");
                                              //TODO refactor into func
                                              //display_set_rssi("denied");
                                              //delay(500);
                        theModem.current_state = STARTUP_RESET;
                    } else if (status == REG_OK_HOME || status == REG_OK_ROAMING) {
                        Serial.printf("\n**** Modem ONLINE ****\n");
                        char buf[9];
                        sprintf(buf,"on:%02d\0",theModem.getSignalQuality());
                        //display_set_rssi(buf);
                        if (first_pass_setup) {
                            first_pass_setup = false;
                            setup_online_modem(); //can this be done when radio is off?
                        }
                        theModem.current_state = ONLINE;
                    }
                    delay(500);

                    break;
                
                case ONLINE:
                    if (waiting_ticks == 0) {
                        log("online");
                    }
                    if (waiting_ticks++ < 120) {
                        Serial.print(".");
                    } else {
                        waiting_ticks = 0;
                        //ensure still connected
                        //theModem.current_state = CHECK_NETWORK;
                        break;
                    }
    
                   // loop_sms();

                   delay(500);
                    break;
    
                default: {
                    Serial.printf("modem: unhandled state:%d\n", theModem.current_state);
                    break;
                }
            }
                //Serial.flush();
           // }
       //     xSemaphoreGive(modem_mutex);
        //}// else {
         //  Serial.println("modem mutex fail");
       // }
        vTaskDelay(10 / portTICK_PERIOD_MS);  // Prevents watchdog timeout
    }
}

/*
void send_wakeup_msg() {
    //imsi.concat(" is online");
    //sms_hello(imsi.c_str());
    //if (millis() - last_get_msgs > 10000) {
    //Serial.println("getting messages");
    //process_incoming_messages();
    //last_get_msgs = millis();
    //}
}
*/

void setup_modem() {
    #ifdef LILYGO_T_SIM7080G
    /*********************************
     *  Initialize power chip,
     *  turn on modem and gps antenna power channel
    ***********************************/
    //use on battery-powered LilyGo, etc.
    pinMode(BOARD_MODEM_PWR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_DTR_PIN, OUTPUT);
    pinMode(BOARD_MODEM_RI_PIN, INPUT);
    
    // Pull down PWRKEY for more than 1 second according to manual requirements
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);
    delay(100);
    digitalWrite(BOARD_MODEM_PWR_PIN, HIGH);
    delay(1100);
    digitalWrite(BOARD_MODEM_PWR_PIN, LOW);

    //affects SIM behav?
    digitalWrite(BOARD_MODEM_DTR_PIN, LOW);
    delay(1000);
    #elif defined(TINY_GSM_MODEM_SIM7070)
    pinMode(PIN_PWR_K, OUTPUT);
    #endif

    setup_power();

    theModem.setup();
    #ifndef DEBUG_AT_COMMAND_MODE
        //create a task that executes the state machine with priority 1 on core 0
        xTaskCreatePinnedToCore(state_machine, "Modem_state_machine", 8192, NULL, 1, &state_machine_task, 0);
    #endif
}

void loop_modem() {
    theModem.maintain();
}