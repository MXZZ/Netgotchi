#pragma once
#if defined(ARDUINO_ARCH_ESP32)
  #include <ESP32Ping.h>
  #define ESPping Ping
#elif defined(ARDUINO_ARCH_ESP8266)
  #include <ESP8266Ping.h>
  #define ESPping Ping
#else
  #error "ESPping compatibility header supports only ESP32/ESP8266."
#endif
