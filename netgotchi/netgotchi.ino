// ===== Fix pack includes (pulsanti + ESP-NOW + Ping) =====
#include "globals.h"   // include pins_local.h, espnow.h, ESPping.h
extern String status;  // usata anche in network.ino

// Netgotchi - lives to protect your network!
// Created by MXZZ https://github.com/MXZZ
// ESP32 port by itsOwen https://github.com/itsOwen
// GNU General Public License v3.0

// ===== Web/WiFi base =====
#if defined(ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
#else
  #error "This code is intended to run on ESP32 or ESP8266 platforms only."
#endif

// Prototipi RGB (definiti in rgb.ino)
void rgbBegin(uint8_t brightness);
void rgbUpdate();

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <WiFiManager.h>  // tzapu
#include <Button2.h>

// ===== Versione =====
const float VERSION = 1.63;

// ===== Disattiva OLED classici (usiamo TFT) =====
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

#define oled_type_ssd1306 0
#define oled_type_sh1106  0
#define oled_type_ssd1305 0
#define NodeMCU_Oled      0

// ===== TFT ST7789 (Waveshare ESP32-C6-LCD-1.47) =====
#define USE_ST7789 1
#include "compat/CompatST7789.h"

// Pinout Waveshare 1.47" (ST7789 172x320)
#define TFT_MOSI  6
#define TFT_SCLK  7
#define TFT_CS    14
#define TFT_DC    15
#define TFT_RST   21
#define TFT_BL    22
#define TFT_W     172
#define TFT_H     320

// --- Logical screen size for ST7789 (landscape 320x172) ---
#if USE_ST7789
  #undef SCREEN_WIDTH
  #undef SCREEN_HEIGHT
  #define SCREEN_WIDTH  320   // larghezza con rotation(1)
  #define SCREEN_HEIGHT 172   // altezza con rotation(1)
#endif
#define HALF_W (SCREEN_WIDTH/2)
#define HALF_H (SCREEN_HEIGHT/2)

// Istanza display fisico
CompatST7789 display(TFT_CS, TFT_DC, TFT_RST, TFT_W, TFT_H);

// ===== Backbuffer tipo "video frame" =====
#define USE_CANVAS_BACKBUFFER 1
#if USE_ST7789 && USE_CANVAS_BACKBUFFER
  // frame 16-bit: 320x172x2 ≈ 110 KB
  GFXcanvas16 canvas(SCREEN_WIDTH, SCREEN_HEIGHT);
  static inline uint16_t col(int c){ return c ? 0xFFFF : 0x0000; } // WHITE/BLACK
#endif

// ===== FTP: disabilitato con stub (niente SD_MMC) =====
#define USE_FTP 0
#if USE_FTP
  #if defined(ESP32)
    #include <ESP32FtpServer.h>
  #elif defined(ESP8266)
    #include <ESP8266FtpServer.h>
  #endif
  FtpServer ftpSrv;
#else
  class FtpServer {
  public:
    void begin(const char* user = nullptr, const char* pass = nullptr) {}
    void init(const char* user = nullptr, const char* pass = nullptr) {}
    void handleFTP() {}
    bool isConnected() const { return false; }
    bool isClientConnected() const { return isConnected(); }  // alias compat
    String getHoneyPotBreachIPandTime() { return ""; }
  };
  FtpServer ftpSrv;
#endif

// ===== Web server =====
#if defined(ESP32)
  WebServer server(80);
#elif defined(ESP8266)
  ESP8266WebServer server(80);
#endif

// ===== Limiter FPS (usato in screens.ino) =====
static unsigned long lastFrame = 0;
const uint16_t FRAME_MS = 33; // ~30 FPS

// ===== Starfield / animazione =====
const int NUM_STARS = 100;
float stars[NUM_STARS][3];
float ufoX = SCREEN_WIDTH / 2;
float ufoY = SCREEN_HEIGHT / 2;
float ufoZ = 0;

// ===== NTP (GMT+2) =====
long timeOffset = 7200;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", timeOffset);

// ===== Timers / stato UI =====
unsigned long previousMillis = 0;
unsigned long previousMillisScan = 0;
unsigned long previousMillisPing = 0;
unsigned long previousMillisSoundAlert = 0;
unsigned long previouslastEvilTwinCheck = 0;

const long interval = 20000;
int screensInterval[] = { 30000, 10000, 10000, 5000, 10000, 5000 };
int i = 0;
int ipnum = 0;
int iprows = 0;
int currentScreen = 0;
int maxScreens = 5;

int max_ip = 255;
bool startScan = false;
const long intervalScan = 60000 * 6;
const long intervalPing = 60000 * 5;
const long intervalSound = 60000 * 2;
const long evilTwinScanInterval = 60000 * 2;

int seconds = 0;
unsigned long currentMillis = 0;

int ips[255] = {};

unsigned long lastPingTime = 0;
bool honeypotTriggered = false;
bool sounds = true;

String externalNetworkStatus = "";
String networkStatus = "";
// stato generico per scansioni / UI
String status = "Idle";

bool scanOnce = true;
String stats = "Not available";

String netgotchiFace = "(-v_v)";
String netgotchiFace2 = "(v_v-)";
String netgotchiFaceBlink = "( .__.)";
String netgotchiFaceSleep = "(-__- )";
String netgotchiFaceSurprised = "(o__o)";
String netgotchiFaceHappy = "(^=^)";
String netgotchiFaceSad = "(T_T)";
String netgotchiFaceSad2 = "(T__T)";
String netgotchiFaceSuspicious = "(>_>)";
String netgotchiFaceSuspicious2 = "(<_<)";
String netgotchiFaceHit = "(x_x)";
String netgotchiFaceHit2 = "(X__X)";
String netgotchiFaceStarryEyed = "(*_*)";
String netgotchiInUfo1 = " <(0-0-0)> ";
String netgotchiInUfo2 = " <(o-o-o)> ";
String netgotchiScreenMessage = "Saving planets!";
String netgotchiCurrentFace = "";
String netgotchiFaceEvilTwin = "(e__t)";
int animState = 0;
int animation = 0;
int max_anim = 1;
long old_seconds = 0;
long serial_info_seconds = 0;
int moveX = 0;
IPAddress currentIP;

const int flashButtonPin = 0;  // GPIO0

WiFiManager wifiManager;

bool useWifiManager = true;
int  wifiManagertimeout = 360;

const char* ssid = "";
const char* password = "";

bool enableNetworkMode     = true;
bool shouldSaveConfig      = false;
bool useButtonToResetFlash = true;   // false per netgotchi pro
bool hasControlsButtons    = false;  // true per netgotchi pro
bool skipLoader            = true;   // false per netgotchi pro
bool debug                 = true;
bool headless              = true;
bool hasDisplay            = true;
bool carouselMode          = true;
bool scheduledRestart      = false;
bool settingMode           = false;
bool securityScanActive    = true;
bool skipFTPScan           = true;
int  vulnerabilitiesFound  = 0;
int  selectedSetting       = 0;

int settingLength = 6;
String settings[] = { "Start AP", "Online Mode", "Airplane Mode", "Start WebInterface", "Restart", "Reset Settings" };

struct Service { const char* name; uint16_t port; };
Service dangerousServices[] = {
  { "Telnet", 23 }, { "FTP", 21 }, { "SSH", 22 }, { "VNC", 5900 },
  { "RDP", 3389 }, { "SMB", 445 }, { "HTTP", 80 }, { "HTTPS", 443 }
};

#define MAX_NETWORKS 10
typedef struct { String ssid; } NetworkInfo;
NetworkInfo knownNetworks[MAX_NETWORKS];
int  numKnownNetworks = 0;
bool evilTwinDetected = false;
bool webInterface     = true;
String headlessStatus = "";

// Broadcast MAC address
uint8_t broadcastAddress[] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };

typedef struct struct_message { char text[250]; } struct_message;

// ---------------- Web UI page ----------------
static const char PROGMEM pagehtml[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Netgotchi</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    body{display:block;height:100%;margin:0;background:#333;color:#fff;font-family:Helvetica}
    canvas{image-rendering:pixelated;zoom:2}
    #headless{background:black;font-size:30px;text-wrap:balance;}
    .led{margin:8px 0 12px}
    .led input[type="range"]{width:120px;vertical-align:middle}
  </style>
</head>
<body>
<h1> Netgotchi </h1>
<br>
<p>Headless display<p>
<p id="headless"></p>
<p>Actual Display</p>
<canvas id="canvas" width="128" height="64"></canvas>
<p>Controls<p>
<div class="buttons">
  <button onclick="sendCommand('left')">Left</button>
  <button onclick="sendCommand('right')">Right</button>
  <button onclick="sendCommand('A')">A</button>
  <button onclick="sendCommand('B')">B</button><br>
  <button onclick="sendCommand('ON')">PIN ON</button>
  <button onclick="sendCommand('OFF')">PIN OFF</button><br>
  <button onclick="sendCommand('TIMEPLUS')">TIME+</button>
  <button onclick="sendCommand('TIMEMINUS')">TIME-</button>
</div>
<p>LED RGB</p>
<div class="led">
  <label><input id="led_en" type="checkbox"> Enable</label>
  &nbsp; Mode:
  <select id="led_mode">
    <option value="auto">auto</option>
    <option value="manual">manual</option>
    <option value="off">off</option>
  </select>
  &nbsp; Brightness:
  <input id="led_b" type="range" min="0" max="255" step="1">
  &nbsp; Color:
  <input id="led_color" type="color" value="#ffffff">
  <button onclick="applyLed()">Apply</button>
</div>
<p>Hosts</p>
<button onclick="getHosts()">Get Hosts Datas</button>
<p id="hosts"></p>
<script>
function updateCanvas(){
  fetch('/matrix').then(r=>r.json()).then(matrix=>{
    const c=document.getElementById('canvas'),x=c.getContext('2d');
    x.fillStyle='black';x.fillRect(0,0,c.width,c.height);
    x.fillStyle='white';
    for(let y=0;y<matrix.length;y++){
      for(let i=0;i<matrix[y].length;i++){
        if(matrix[y][i]===1){x.fillRect(i,y,1,1);}
      }
    }
  }).catch(console.error);
}

function sendCommand(cmd){
  fetch('/command/'+cmd).then(r=>r.text()).then(console.log).catch(console.error);
}

function getHosts(){
  fetch('/hosts').then(r=>r.text()).then(t=>{
    document.getElementById('hosts').innerHTML=t;
  }).catch(console.error);
}

function getHeadlessStatus(){
  fetch('/headless').then(r=>r.text()).then(t=>{
    document.getElementById('headless').innerHTML=t;
  }).catch(console.error);
}

/* ======== LED RGB ======== */
function hexToRgb(hex){
  const m = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
  return m ? {r: parseInt(m[1],16), g: parseInt(m[2],16), b: parseInt(m[3],16)} : {r:255,g:255,b:255};
}

function refreshLed(){
  fetch('/led/state').then(r=>r.json()).then(s=>{
    document.getElementById('led_en').checked = !!s.enabled;
    document.getElementById('led_mode').value = s.mode;           // "auto" | "manual" | "off"
    document.getElementById('led_b').value    = s.brightness;     // 0..255
    const hex = "#"+[s.r,s.g,s.b].map(v=>('0'+v.toString(16)).slice(-2)).join('');
    document.getElementById('led_color').value = hex;
  }).catch(console.error);
}

function applyLed(){
  const en   = document.getElementById('led_en').checked ? 1 : 0;
  const mode = document.getElementById('led_mode').value;
  const b    = document.getElementById('led_b').value;
  const col  = hexToRgb(document.getElementById('led_color').value);

  fetch(`/led?enable=${en}&mode=${encodeURIComponent(mode)}&b=${b}`)
    .then(()=>{
      if(mode === 'manual'){
        return fetch(`/led/color?r=${col.r}&g=${col.g}&b=${col.b}`);
      }
    })
    .then(refreshLed)
    .catch(console.error);
}
/* ========================= */

setInterval(updateCanvas,2000);
setInterval(getHeadlessStatus,2000);

// iniziali
updateCanvas();
getHeadlessStatus();
refreshLed();
</script>
</body></html>
)rawliteral";

// ---------------- Wrapper funzioni display (canvas backbuffer) ----------------
void displayPrintln(String line = "") {
#if USE_ST7789 && USE_CANVAS_BACKBUFFER
  if (hasDisplay) canvas.println(line);
#else
  if (hasDisplay) display.println(line);
#endif
}
void displaySetCursor(int x, int y) {
#if USE_ST7789 && USE_CANVAS_BACKBUFFER
  if (hasDisplay) canvas.setCursor(x, y);
#else
  if (hasDisplay) display.setCursor(x, y);
#endif
}
void displayPrint(String line) {
#if USE_ST7789 && USE_CANVAS_BACKBUFFER
  if (hasDisplay) canvas.print(line);
#else
  if (hasDisplay) display.print(line);
#endif
}
void displayClearDisplay() {
#if USE_ST7789 && USE_CANVAS_BACKBUFFER
  if (hasDisplay) canvas.fillScreen(0x0000);
#else
  if (hasDisplay) display.clearDisplay();
#endif
}
void displaySetSize(int size) {
#if USE_ST7789 && USE_CANVAS_BACKBUFFER
  if (hasDisplay) canvas.setTextSize(size);
#else
  if (hasDisplay) display.setTextSize(size);
#endif
}
void displaySetTextColor(int color) {
#if USE_ST7789 && USE_CANVAS_BACKBUFFER
  if (hasDisplay) canvas.setTextColor(col(color));
#else
  if (hasDisplay) display.setTextColor(color);
#endif
}
void displayPrintDate(const char* format, int d, int m, int y) {
#if USE_ST7789 && USE_CANVAS_BACKBUFFER
  if (hasDisplay) canvas.printf(format, d, m, y);
#else
  if (hasDisplay) display.printf(format, d, m, y);
#endif
}
void displayDrawLine(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t color) {
#if USE_ST7789 && USE_CANVAS_BACKBUFFER
  if (hasDisplay) canvas.drawLine(x0,y0,x1,y1,col(color));
#else
  if (hasDisplay) display.drawLine(x0,y0,x1,y1,color);
#endif
}
void displayDrawCircle(uint16_t x,uint16_t y,uint16_t r,uint16_t color) {
#if USE_ST7789 && USE_CANVAS_BACKBUFFER
  if (hasDisplay) canvas.drawCircle(x,y,r,col(color));
#else
  if (hasDisplay) display.drawCircle(x,y,r,color);
#endif
}
void displayDrawPixel(uint16_t x,uint16_t y,uint16_t color) {
#if USE_ST7789 && USE_CANVAS_BACKBUFFER
  if (hasDisplay) canvas.drawPixel(x,y,col(color));
#else
  if (hasDisplay) display.drawPixel(x,y,color);
#endif
}
void displayFillRect(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t color){
#if USE_ST7789 && USE_CANVAS_BACKBUFFER
  if (hasDisplay) canvas.fillRect(x,y,w,h,col(color));
#else
  if (hasDisplay) display.fillRect(x,y,w,h,color);
#endif
}

// === "Present" della frame: push unico su TFT ===
void displayDisplay() {
#if USE_ST7789 && USE_CANVAS_BACKBUFFER
  if (!hasDisplay) return;
  auto &tft = display.tft();
  tft.startWrite();
  tft.setAddrWindow(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  tft.writePixels((uint16_t*)canvas.getBuffer(), (uint32_t)SCREEN_WIDTH * (uint32_t)SCREEN_HEIGHT, true);
  tft.endWrite();
#else
  if (hasDisplay) display.display();
#endif
}

// OLED avevano framebuffer; ST7789 no → per WebUI “matrix” ritorniamo NULL
uint8_t* displayGetBuffer() {
#ifdef USE_ST7789
  return NULL;
#else
  if (hasDisplay) return display.getBuffer();
  else return NULL;
#endif
}

// ---------------- Serial helpers ----------------
void SerialPrintLn(String message){ if (debug) Serial.println(message); }
void SerialPrintLn(int message)  { if (debug) Serial.println(message);  }

// ---------------- PROTOTIPI ----------------
void displayInit();
void loaderSetup();
void loader();
void checkOfflineMode();
void netgotchiIntro();
void networkInit();
void saveCurrentNetworkInfos();
void buttonsInit();
void initStars();
void networkFunctionsLoop();
void nextScreen();
void displaySettings();
void screenAnimations();
void NetworkStats();
void displayIPS();
void displayNetgotchiStats();
void displayRippleSpace();
void buttonLoops();
void controlsButtonLoop();
void countdownToRestart();

// ---------------- Setup / Loop ----------------
void setup() {
  Serial.begin(115200);
  if (NodeMCU_Oled) Wire.begin(14, 12); // disattivo: NodeMCU_Oled=0
  displayInit();     // TFT init
  loaderSetup();
}

void loop() {
  loader();
}

// ---------------- Headless info ----------------
void headlessInfo() {
  if (seconds - serial_info_seconds > 1) {
    serial_info_seconds = seconds;
    headlessStatus = netgotchiCurrentFace + " Honeypot:" + (honeypotTriggered ? "breached" : "OK")
                   + " EvilTwin:" + (evilTwinDetected ? "detected" : "OK")
                   + " Host-Found:" + String(ipnum)
                   + " Vulnerabilities:" + String(vulnerabilitiesFound);
    SerialPrintLn(headlessStatus);
  }
}

// ---------------- Netgotchi lifecycle ----------------
void netgotchi_setup() {
  displayInit();
  rgbBegin(40);   // avvia RGB, brightness 0..255
  if (hasControlsButtons) checkOfflineMode();
  netgotchiIntro();

  if (enableNetworkMode) {
    networkInit();
    saveCurrentNetworkInfos();
  }
  if (useButtonToResetFlash) pinMode(flashButtonPin, INPUT_PULLUP);
  if (hasControlsButtons) buttonsInit();

  initStars();
}

void netgotchi_loop() {
  currentMillis = millis();
  seconds = currentMillis / 1000;

  if (enableNetworkMode) networkFunctionsLoop();

  if (!settingMode && carouselMode && (currentMillis - previousMillis >= screensInterval[currentScreen])) {
    previousMillis = currentMillis;
    nextScreen();
  }

  if (settingMode) displaySettings();

  if (!settingMode) {
    if (currentScreen == 0) screenAnimations();
    if (currentScreen == 1) NetworkStats();
    if (currentScreen == 2) displayIPS();
    if (currentScreen == 3) displayNetgotchiStats();
    if (currentScreen == 4) displayRippleSpace();
  }

  if (useButtonToResetFlash) buttonLoops();
  if (hasControlsButtons)    controlsButtonLoop();
  if (scheduledRestart)      countdownToRestart();

  if (headless) headlessInfo();
  rgbUpdate();    // aggiorna animazioni RGB in base allo stato
  delay(15);
}

// ---------------- Inizializzazione DISPLAY (TFT ST7789) ----------------
void displayInit() {
#ifdef USE_ST7789
  // SPI esplicita sui pin del Waveshare (nessun MISO)
  SPI.begin(TFT_SCLK, /*MISO=*/-1, TFT_MOSI);

  display.begin();          // init ST7789 172x320
  display.setRotation(1);   // 1 o 3 = landscape, 0/2 = portrait
  display.tft().setSPISpeed(36000000); // 36 MHz; se vedi artefatti scendi a 27 MHz

  // display pulito (sul backbuffer)
  displayClearDisplay();
  #if USE_ST7789 && USE_CANVAS_BACKBUFFER
  canvas.setTextWrap(false);   // evita che i testi “sbordino/ritornino a capo”
  #endif

  // retroilluminazione (attiva HIGH su GPIO22)
  display.setBacklightPin(TFT_BL, true);
  display.backlight(true);

  // riallinea le animazioni al centro del nuovo canvas
  ufoX = HALF_W;
  ufoY = HALF_H;
#else
  // --- Vecchia init OLED ---
  // display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  // display.clearDisplay();
#endif
}