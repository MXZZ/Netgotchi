# Netgotchi (ESP32-C6 + ST7789) — Modifiche

Questo documento sostituisce il README originale e riepiloga **tutte le modifiche** effettuate per far funzionare Netgotchi su **Waveshare ESP32‑C6‑LCD‑1.47"** (display **ST7789 172×320**) con **Arduino IDE**.

---

## Obiettivi del porting
- Passaggio da OLED I²C a **TFT ST7789** con wrapper compatibile (API basilari Adafruit SSD1306 emulate).
- Adeguamento **ESP‑NOW** alle API nuove del core **ESP32 v3.x** (IDF 5.x).
- **FTP disabilitato** tramite **stub** (nessuna dipendenza `SD_MMC`).
- Escluso il modulo **deauther** (solo ESP8266) con **stub** sicuri.
- Fix vari: pin pulsanti/buzzer sempre visibili, rimozione doppie `displayInit()`, WebUI “matrix” compatibile con TFT (senza framebuffer), ping min/avg/max opzionale.

---

## Requisiti
**Scheda/Core**
- Board: `ESP32C6 Dev Module`
- Core: **ESP32 Arduino v3.x**

**Librerie (Gestione librerie)**
- Adafruit GFX Library  
- Adafruit ST7735 and ST7789 Library  
- WiFiManager (tzapu)  
- Button2 (Lennart Hennigs)  
- NTPClient  
- ESP32Ping *(opzionale, per min/avg/max; altrimenti solo avg)*

---

## File aggiunti o aggiornati

### Nella cartella dello sketch
- `globals.h` → include **unico** per tutti i `.ino` (importa `pins_local.h`, `espnow.h`, `ESPping.h`). **Metterlo come PRIMA riga** in ogni `.ino` che usa pulsanti/ESP‑NOW/Ping.
- `pins_local.h` → definisce `BTN_A/B/LEFT/RIGHT`, `BUZZER_PIN`, `EXT_PIN_16` se mancanti.
- `espnow.h` → wrapper compatibilità: 
  - `esp_now_set_self_role(...)` → **no‑op** su ESP32
  - Adattatore per **callback legacy** `void(mac,data,len)` verso la nuova firma con `esp_now_recv_info*`
  - **Include guard** robuste (v5) per evitare redefinition.
- `ESPping.h` → mappa verso ESP32Ping/ESP8266Ping.
- `netgotchi/compat/CompatST7789.h` → aggiornato: `drawLine`, `drawCircle`, `setTextColor(fg,bg)` (mappa 0/1 a BLACK/WHITE), metodi base usati dal progetto.

### Modifiche principali ai sorgenti
- **`netgotchi.ino`**
  - Aggiunti:
    ```cpp
    #include "globals.h"
    #define USE_ST7789 1
    #include "compat/CompatST7789.h"
    ```
  - Pin Waveshare ST7789:
    ```cpp
    #define TFT_MOSI 6
    #define TFT_SCLK 7
    #define TFT_CS   14
    #define TFT_DC   15
    #define TFT_RST  21
    #define TFT_BL   22
    #define TFT_W    172
    #define TFT_H    320
    ```
  - Istanza:
    ```cpp
    CompatST7789 display(TFT_CS, TFT_DC, TFT_RST, TFT_W, TFT_H);
    ```
  - `displayInit()` (TFT):
    ```cpp
    SPI.begin(TFT_SCLK, /*MISO=*/-1, TFT_MOSI);
    display.begin();
    display.setRotation(1);         // landscape
    display.clearDisplay();
    display.setBacklightPin(TFT_BL, true);
    display.backlight(true);
    ```
  - **FTP disabilitato** con stub:
    ```cpp
    #define USE_FTP 0
    #if USE_FTP
      // include FTP reali...
    #else
      class FtpServer {
      public:
        void begin(const char* u=nullptr,const char* p=nullptr) {}
        void init(const char* u=nullptr,const char* p=nullptr) {}
        void handleFTP() {}
        bool isConnected() const { return false; }
        bool isClientConnected() const { return isConnected(); } // alias
        String getHoneyPotBreachIPandTime() { return ""; }
      };
      FtpServer ftpSrv;
    #endif
    ```
  - WebUI “matrix”: su ST7789 non c’è framebuffer → `displayGetBuffer()` ritorna `NULL`.

- **`screens.ino`**
  - Rimossa/condizionata la **doppia** `displayInit()` (mantieni quella in `netgotchi.ino`). Esempio:
    ```cpp
    #ifdef USE_ST7789
      // niente displayInit qui
    #else
    void displayInit() { /* init OLED */ }
    #endif
    ```
  - `getPixelAt()`: per ST7789 restituisce 0 (nero) per evitare crash della WebUI.

- **`network.ino`**
  - Var globale `status` è definita in `netgotchi.ino` → aggiunto in cima:
    ```cpp
    #include "globals.h"
    extern String status;
    ```
  - Con stub FTP, se usi `isClientConnected()` resta compatibile (alias verso `isConnected()`).

- **`ctrlgotchi.ino` / `textgotchi.ino` (ESP‑NOW)**
  - In cima:
    ```cpp
    #include "globals.h"
    ```
  - Sostituita la vecchia firma di **add peer**:
    ```cpp
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, broadcastAddress, 6);
    peer.ifidx   = WIFI_IF_STA;      // o WIFI_IF_AP
    peer.channel = 1;                // opzionale: allinea al tuo Wi‑Fi
    peer.encrypt = false;
    esp_now_add_peer(&peer);
    ```
  - Niente più `esp_now_set_self_role(ESP_NOW_ROLE_COMBO)` (il wrapper lo rende no‑op).
  - Le callback “vecchie” `void (*)(uint8_t*,uint8_t*,uint8_t)` restano valide grazie all’adattatore.

- **`deauthergotchi.ino` (solo ESP8266)**
  - Escluso su ESP32‑C6. Due alternative:
    - Rinomina il file (es. `deauthergotchi.ino.disabled`) **oppure**
    - Aggiungi all’inizio:
      ```cpp
      #if !defined(ESP8266)
      void deauthergotchi_setup() {}
      void deauthergotchi_loop()  {}
      #else
      // codice originale 8266...
      #endif
      ```
  - Se `loader.ino` richiama quelle funzioni, gli **stub** evitano errori.

---

## Istruzioni di build
1. Copia i file aggiuntivi (`globals.h`, `pins_local.h`, `espnow.h`, `ESPping.h`) nella **cartella dello sketch**.
2. Sostituisci `netgotchi/compat/CompatST7789.h` con quello aggiornato.
3. Assicurati che ogni `.ino` che usa pulsanti/ESP‑NOW/Ping inizi con:
   ```cpp
   #include "globals.h"
   ```
4. Escludi o proteggi `deauthergotchi.ino` come sopra.
5. (Opzionale) Installa **ESP32Ping** per avere min/avg/max nelle statistiche; altrimenti in `screens.ino` usa solo:
   ```cpp
   stats = "\n avg: " + String(Ping.averageTime()) + "ms";
   ```
6. Board: **ESP32C6 Dev Module** → Compila e carica.

---

## Troubleshooting
- **API ESP‑NOW vecchie** (`ESP_NOW_ROLE_COMBO`, `esp_now_set_self_role`) → assicurati che **tutti** i `.ino` includano **prima** `globals.h` e che **nessuno** includa `espnow.h` direttamente.
- **`esp_now_add_peer(..., ...)` non compila** → usa la versione a **struct** `esp_now_peer_info_t` (vedi sopra).
- **`minTime()/maxTime()` mancanti** → installa **ESP32Ping** o limita agli `averageTime()`.
- **Errori `SD_MMC`** → FTP **disabilitato** (stub in `netgotchi.ino`).
- **WebUI canvas vuoto** → è normale su ST7789 (niente framebuffer per la “matrix” 128×64).

---

## Pin display (Waveshare ESP32‑C6‑LCD‑1.47)

| Segnale | GPIO |
|:--|--:|
| MOSI | 6 |
| SCLK | 7 |
| CS   | 14 |
| DC   | 15 |
| RST  | 21 |
| BL   | 22 |

> Nessun MISO necessario per ST7789.

---

## Note legali
Le funzioni “deauther” sono **disabilitate** su ESP32‑C6 e possono essere **illegali** in molti paesi.

---

## Crediti
- Progetto originale: **MXZZ / Netgotchi**  
- Port ESP32: **itsOwen**  
- Port ESP32‑C6 + ST7789 + fix Arduino IDE: **questa patch** (GPLv3, coerente con il progetto).
