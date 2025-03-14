#pragma once

// analog button array (voltage divider)
#ifdef CONFIG_IDF_TARGET_ESP32S3
#define ANALOG_BTN_PIN  A0
#else
#define ANALOG_BTN_PIN  A7
#endif