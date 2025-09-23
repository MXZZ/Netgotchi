#ifndef NETGOTCHI_ESPNOW_WRAPPER_H
#define NETGOTCHI_ESPNOW_WRAPPER_H

#if defined(ARDUINO_ARCH_ESP32)
  #include <WiFi.h>
  #include <esp_now.h>

  #ifndef esp_now_set_self_role
    #define esp_now_set_self_role(x) ((void)0)
  #endif
  #ifndef ESP_NOW_ROLE_COMBO
    #define ESP_NOW_ROLE_COMBO 0
  #endif

  typedef void (*espnow_legacy_recv_cb_t)(uint8_t*, uint8_t*, uint8_t);

  static inline espnow_legacy_recv_cb_t& __espnow_legacy_cb_ref() {
    static espnow_legacy_recv_cb_t cb = nullptr;
    return cb;
  }

  static inline void __espnow_new_cb(const esp_now_recv_info * info, const uint8_t *data, int len) {
    espnow_legacy_recv_cb_t cb = __espnow_legacy_cb_ref();
    if (!cb) return;
    uint8_t mac[6] = {0};
    if (info && info->src_addr) memcpy(mac, info->src_addr, 6);
    cb(mac, (uint8_t*)data, (uint8_t)len);
  }

  static inline esp_err_t esp_now_register_recv_cb_legacy(espnow_legacy_recv_cb_t cb){
    __espnow_legacy_cb_ref() = cb;
    return esp_now_register_recv_cb(__espnow_new_cb);
  }

  #ifdef esp_now_register_recv_cb
    #undef esp_now_register_recv_cb
  #endif
  #define esp_now_register_recv_cb(cb) esp_now_register_recv_cb_legacy(cb)

#elif defined(ARDUINO_ARCH_ESP8266)
  #include <ESP8266WiFi.h>
  #include <espnow.h>
#else
  #error "ESP-NOW wrapper supports only ESP32/ESP8266."
#endif

#endif // NETGOTCHI_ESPNOW_WRAPPER_H
