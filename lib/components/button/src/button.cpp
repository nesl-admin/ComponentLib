#include <Arduino.h>
#include "button.h"

bool pressed = false;
int last_btn_press_timestamp = 0;
void (*button1_cb)();
void (*button2_cb)();
void (*button3_cb)();
void (*button4_cb)();

void button1_pushed() {
    return (*button1_cb)();
}
void button2_pushed() {
    return (*button2_cb)();
}
void button3_pushed() {
    return (*button3_cb)();
}
void button4_pushed() {
    return (*button4_cb)();
}

void setup_buttons() {
   pinMode(ANALOG_BTN_PIN, INPUT_PULLUP);
}

void setup_button_callbacks(void (*cb1)(), void (cb2)(), void (cb3)(), void (*cb4)()) {
    button1_cb = cb1;
    button2_cb = cb2;
    button3_cb = cb3;
    button4_cb = cb4;
}

bool interval_buttons() {
    if (millis() - last_btn_press_timestamp > 250) {
        last_btn_press_timestamp = millis();
        return true;
    }
    return false;
}

//Voltage divider button array using 1K resistors:
//NOTE: differs on EnergyTracker2 breadboard
//V      BTN
//------------
// 4095   none
// 0      1
// 1975   2
// 2660   3
// 3015   4
void loop_buttons() {
    //if (interval_buttons()) {
        int val = analogRead(ANALOG_BTN_PIN);
        //int cnt=0;
        //Serial.printf("A0 VCC: %d\n", val);
        if (val >= 4000){
            //no buttons - reset
            pressed = false;
            return;
        
        //BUTTON 4
        } else if (val>3800 && !pressed) {
            Serial.printf("BUTTON 4\n");
            button4_pushed();
            pressed = true;

        //BUTTON 3
        } else if (val > 3300 && !pressed) {
            Serial.printf("BUTTON 3\n");
            button3_pushed();
            pressed = true;
    
        //BUTTON 2
        } else if (val > 2500 && !pressed) {
            Serial.printf("BUTTON 2\n");
            button2_pushed();
            pressed = true;

        //BUTTON 1
        } else if (val > 200 && !pressed ) {
            Serial.printf("BUTTON 1\n");
            button1_pushed();
            pressed = true;
        }
    //}
}

bool toggle_halt = true;
void display_next_frame() {
    //setLED_RED(3);
    //getUI()->nextFrame();
    if (toggle_halt) {
        //getUI()->haltFrame();
    } else {
        //getUI()->nextFrame(); //resume auto-frames
    }
    toggle_halt = !toggle_halt;
    //getUI()->forceUpdate();
    //delay(500);
    //setLED_OFF(3);
}

void wifi_join_and_mqtt_publish() {

}