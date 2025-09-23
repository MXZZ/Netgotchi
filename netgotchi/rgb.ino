#include <Adafruit_NeoPixel.h>
#include <WiFi.h>   // per WiFi.status()

// --- pin/count (Waveshare ESP32-C6-LCD-1.47) ---
#ifndef RGB_PIN
  #define RGB_PIN   8
#endif
#ifndef RGB_COUNT
  #define RGB_COUNT 1
#endif

// --- variabili globali controllabili da WebUI ---
bool     rgb_enabled      = true;
uint8_t  rgb_brightness   = 60;     // 0..255
int      rgb_mode         = RGB_MODE_AUTO;
uint8_t  rgb_manual_r     = 255, rgb_manual_g = 255, rgb_manual_b = 255;

Adafruit_NeoPixel rgb(RGB_COUNT, RGB_PIN, NEO_GRB + NEO_KHZ800);

// extern da altri file (usati per lo stato)
extern bool honeypotTriggered;
extern bool evilTwinDetected;
extern int  vulnerabilitiesFound;
extern bool startScan;
extern bool useWifiManager;

// ------------ helpers --------------
static inline void rgbAll(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < RGB_COUNT; i++)
    rgb.setPixelColor(i, rgb.Color(r, g, b));
  rgb.show();
}
static inline void rgbOff() { rgb.clear(); rgb.show(); }

// onda 0..1
static inline float wave01(uint16_t period_ms) {
  uint16_t t = millis() % period_ms;
  return (t < period_ms/2) ? (t/(period_ms/2.0f)) : ((period_ms - t)/(period_ms/2.0f));
}

// ------------ API invocabili da altri file --------------
void rgbBegin(uint8_t brightness) {
  rgb.begin();
  rgb.setBrightness(brightness);
  rgb_brightness = brightness;
  if (rgb_enabled) rgbAll(0, 32, 0); else rgbOff();
}

void rgbSetBrightness(uint8_t b) {
  rgb_brightness = b;
  rgb.setBrightness(b);
  rgb.show();
}

void rgbEnable(bool en) {
  rgb_enabled = en;
  if (!en) rgbOff();
}

// loop non bloccante: sceglie colore/animazione in base allo stato
void rgbUpdate() {
  if (!rgb_enabled || rgb_mode == RGB_MODE_OFF) { rgbOff(); return; }

  if (rgb_mode == RGB_MODE_MANUAL) {
    rgbAll(rgb_manual_r, rgb_manual_g, rgb_manual_b);
    return;
  }

  // ---- modalità AUTO: priorità dei problemi > config > ok ----
  // 1) Problemi / intrusioni
  if (honeypotTriggered) {
    // flash rosso/blu veloce
    bool phase = ((millis()/120) % 2) == 0;
    if (phase) rgbAll(255, 0, 0); else rgbAll(0, 0, 255);
    return;
  }
  if (evilTwinDetected) {
    // respiro blu
    uint8_t v = (uint8_t)(wave01(900) * 255);
    rgbAll(0, 0, v);
    return;
  }
  if (vulnerabilitiesFound > 0) {
    // respiro rosso
    uint8_t v = (uint8_t)(wave01(1000) * 255);
    rgbAll(v, 0, 0);
    return;
  }

  // 2) Connessione/config
  if (useWifiManager || WiFi.status() != WL_CONNECTED || startScan) {
    // respiro arancio/giallo
    float p = wave01(1100);
    uint8_t r = 255;
    uint8_t g = (uint8_t)(160 + (95 * p)); // 160..255 (arancio→giallo)
    rgbAll(r, g, 0);
    return;
  }

  // 3) Tutto OK → bianco/verde (respiro morbido)
  float p = wave01(1400);
  uint8_t g = (uint8_t)(80 + 175 * p);      // 80..255
  uint8_t r = (uint8_t)(40 + 120 * p/2.0);  // leggero bianco
  uint8_t b = (uint8_t)(40 + 120 * p/2.0);
  rgbAll(r, g, b);
}
