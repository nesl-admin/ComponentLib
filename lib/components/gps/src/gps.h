#pragma once
#include <pins_gps.h>
#include <TinyGPSPlus.h>
TinyGPSPlus* get_gps();
void setup_gps();
void loop_gps();
