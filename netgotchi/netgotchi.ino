// Netgotchi - lives to protect your network!
// Created by MXZZ https://github.com/MXZZ
// ESP32 port by itsOwen https://github.com/itsOwen
// GNU General Public License v3.0

// Include necessary libraries based on the board type
#ifdef ESP32
#include <WiFi.h>
#include <ESP32FtpServer.h>
#include <WebServer.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266FtpServer.h>
#include <ESP8266WebServer.h>
#else
#error "This code is intended to run on ESP32 or ESP8266 platforms only."
#endif

#include <ESPping.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiManager.h>  // Include the WiFiManager library
#include <Button2.h>

const float VERSION = 1.5;

//Oled Screen Selectors
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

//select to 1 which oled driver you have , default is ssd1306 , ssh1106 for netgotchi pro 
#define oled_type_ssd1306 1
#define oled_type_sh1106 0
#define oled_type_ssd1305 0

#define BTN_RIGHT 13
#define BTN_LEFT 12 
#define BTN_A 2
#define BTN_B 0
//#define BUZZER_PIN 15 //for netgotchi pro
//#define BUZZER_PIN 13 //for netgotchis v2
#define BUZZER_PIN 13
#define EXT_PIN_16 16  // D0 on pro

#if oled_type_ssd1305
#include <Adafruit_SSD1305.h>
Adafruit_SSD1305 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#elif oled_type_sh1106
#include <Adafruit_SH110X.h>
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#elif oled_type_ssd1306
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

#ifdef ESP32
WebServer server(80);
#elif defined(ESP8266)
ESP8266WebServer server(80);
#endif

FtpServer ftpSrv;  // Create an instance of the FTP server

const int NUM_STARS = 100;
float stars[NUM_STARS][3];
float ufoX = SCREEN_WIDTH / 2;
float ufoY = SCREEN_HEIGHT / 2;
float ufoZ = 0;
long timeOffset = 7200;  // offset for GMT+2 from https://www.epochconverter.com/timezones

String status = "Idle";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", timeOffset);

unsigned long previousMillis = 0;
unsigned long previousMillisScan = 0;
unsigned long previousMillisPing = 0;
unsigned long previousMillisSoundAlert = 0;
unsigned long previouslastEvilTwinCheck = 0;

const long interval = 20000;
//delay interval for each screen
int screensInterval[] = { 30000, 10000, 10000, 5000, 10000, 5000 };
int i = 0;
int ipnum = 0;   // display counter
int iprows = 0;  // ip rows
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

const int flashButtonPin = 0;  // GPIO0 is connected to the flash button

WiFiManager wifiManager;

bool useWifiManager = true;
int wifiManagertimeout = 360;  // seconds to run for

const char* ssid = "";
const char* password = "";

bool enableNetworkMode = true;
bool shouldSaveConfig = false;
bool useButtonToResetFlash = true;//false for netgotchi pro 
bool hasControlsButtons = false; //true for netgotchi pro 
bool debug = true;
bool headless = true;
bool hasDisplay = true;
bool carouselMode = true;
bool scheduledRestart = false;
bool settingMode = false;

bool securityScanActive = true;
bool skipFTPScan = true;
int vulnerabilitiesFound = 0;
int selectedSetting = 0;

int settingLength = 6;
String settings[] = { "Start AP", "Online Mode", "Airplane Mode", "Start WebInterface", "Stop WebInterface", "Reset Settings" };

struct Service {
  const char* name;
  uint16_t port;
};

Service dangerousServices[] = {
  { "Telnet", 23 },
  { "FTP", 21 },
  { "SSH", 22 },
  { "VNC", 5900 },
  { "RDP", 3389 },
  { "SMB", 445 },
  { "HTTP", 80 },
  { "HTTPS", 443 }
};


#define MAX_NETWORKS 10
typedef struct {
  String ssid;
} NetworkInfo;

NetworkInfo knownNetworks[MAX_NETWORKS];
int numKnownNetworks = 0;
bool evilTwinDetected = false;
bool webInterface = true;
String headlessStatus = "";

static const char PROGMEM pagehtml[] = R"rawliteral( 
<!DOCTYPE html>
<html>
<head>
    <title>Netgotchi</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body {
            display: block;
            height: 100%;
            margin: 0;
            background-color: #333;
            color:white;
            font-family : Helvetica
        }
        canvas {
            image-rendering: pixelated; /* Ensures the pixels remain sharp when scaled */
            zoom: 2;
            }
        #headless{
          background: black;
          font-size: 30px;
          text-wrap: balance;
        }
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
        <button onclick="sendCommand('B')">B</button>
        <button onclick="sendCommand('ON')">PIN ON</button>
        <button onclick="sendCommand('OFF')">PIN OFF</button>
</div>
<p>Hosts</p>
<button onclick="getHosts()">Get Hosts Datas</button>
<p id="hosts"></p>
    <script>
        function updateCanvas() {
            fetch('/matrix')
                .then(response => response.json())
                .then(matrix => {
                    const canvas = document.getElementById('canvas');
                    const ctx = canvas.getContext('2d');
                    ctx.fillStyle = 'black';
                    ctx.fillRect(0, 0, canvas.width, canvas.height);
                    ctx.fillStyle = 'white';
                    for (let y = 0; y < matrix.length; y++) {
                        for (let x = 0; x < matrix[y].length; x++) {
                            if (matrix[y][x] === 1) {
                                ctx.fillRect(x, y, 1, 1);
                            }
                        }
                    }
                });
        }

        function sendCommand(command) {
            fetch('/command/' + command)
                .then(response => response.text())
                .then(data => console.log(data))
                .catch(error => console.error('Error:', error));
        }

        function getHosts() {
            fetch('/hosts' )
                .then(response => response.text())
                .then(response => document.getElementById('hosts').innerHTML= response)
                .catch(error => console.error('Error:', error));
        }
         function getHeadlessStatus() {
            fetch('/headless' )
                .then(response => response.text())
                .then(response => document.getElementById('headless').innerHTML= response)
                .catch(error => console.error('Error:', error));
        }

        // Update the canvas every 2 second
        setInterval(updateCanvas, 2000);

        setInterval(getHeadlessStatus, 2000);

        // Initial update
        updateCanvas();
    </script>
</body></html>
)rawliteral";


//wrapper functions for display
void displayPrintln(String line = "") {
  if (hasDisplay) display.println(line);
}
void displaySetCursor(int x, int y) {
  if (hasDisplay) display.setCursor(x, y);
}
void displayPrint(String line) {
  if (hasDisplay) display.print(line);
}
void displayClearDisplay() {
  if (hasDisplay) display.clearDisplay();
}
void displaySetSize(int size) {
  if (hasDisplay) display.setTextSize(size);
}
void displaySetTextColor(int color) {
  if (hasDisplay) display.setTextColor(color);
}
void displayPrintDate(const char* format, int day, int month, int year) {
  if (hasDisplay) display.printf(format, day, month, year);
}
void displayDisplay() {
  if (hasDisplay) display.display();
}
void displayDrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
  if (hasDisplay) display.drawLine(x0, y0, x1, y1, color);
}
void displayDrawCircle(uint16_t x, uint16_t y, uint16_t radius, uint16_t color) {
  if (hasDisplay) display.drawCircle(x, y, radius, color);
}
void displayDrawPixel(uint16_t x, uint16_t y, uint16_t color) {
  if (hasDisplay) display.drawPixel(x, y, color);
}


uint8_t* displayGetBuffer() {
  if (hasDisplay) return display.getBuffer();
  else return NULL;
}

void SerialPrintLn(String message) {
  if (debug) Serial.println(message);
}
void SerialPrintLn(int message) {
  if (debug) Serial.println(message);
}


void setup() {
  Serial.begin(115200);

  displayInit();
  netgotchiIntro();

  if (enableNetworkMode) {
    networkInit();
    saveCurrentNetworkInfos();
  }
  if (useButtonToResetFlash) pinMode(flashButtonPin, INPUT_PULLUP);
  if (hasControlsButtons) buttonsInit();

  initStars();
}

void loop() {
  currentMillis = millis();
  seconds = currentMillis / 1000;

  //main netgotchi network functionalities
  if (enableNetworkMode) networkFunctionsLoop();

  //display carousel, each screen has a different duration
  if (!settingMode && carouselMode && (currentMillis - previousMillis >= screensInterval[currentScreen])) {
    previousMillis = currentMillis;
    nextScreen();
  }

  //settings
  if (settingMode) displaySettings();

  if (!settingMode) {
    if (currentScreen == 0) screenAnimations();
    if (currentScreen == 1) NetworkStats();
    if (currentScreen == 2) displayIPS();
    if (currentScreen == 3) displayNetgotchiStats();
    if (currentScreen == 4) displayRippleSpace();
  }

  //button loops
  if (useButtonToResetFlash) buttonLoops();
  if (hasControlsButtons) controlsButtonLoop();
  if (scheduledRestart) countdownToRestart();

  //headless infos
  if (headless) headlessInfo();



  delay(15);
}

void headlessInfo() {
  if (seconds - serial_info_seconds > 1) {
    serial_info_seconds = seconds;
    headlessStatus = netgotchiCurrentFace + " Honeypot:" + (honeypotTriggered ? "breached" : "OK") + " EvilTwin:" + (evilTwinDetected ? "detected" : "OK") + " Host-Found:" + String(ipnum) + " Vulnerabilities:" + String(vulnerabilitiesFound);
    SerialPrintLn(headlessStatus);
  }
}


