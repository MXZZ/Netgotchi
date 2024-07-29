// Netgotchi - lives to protect your network!
// Created by MXZZ https://github.com/MXZZ
// ESP32 port by itsOwen https://github.com/itsOwen
// GNU General Public License v3.0

// Include necessary libraries based on the board type
#ifdef ESP32
  #include <WiFi.h>
  #include <ESP32FtpServer.h> 
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266FtpServer.h> 
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

const float VERSION = 1.2;

//Oled Screen Selectors
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

//select to 1 which oled driver you have , default is ssd1306
#define oled_type_ssd1306 1
#define oled_type_sh1106 0
#define oled_type_ssd1305 0

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
int i = 0;
int ipnum = 0;   // display counter
int iprows = 0;  // ip rows
int currentScreen = 0;
int max_ip = 255;
bool startScan = false;
const long intervalScan = 60000 * 4;
const long intervalPing = 60000 * 5;
const long intervalSound = 60000 * 2;
const long evilTwinScanInterval = 60000 * 2;


int seconds = 0;

int ips[255] = {};

bool pingScanDetected = false;
unsigned long lastPingTime = 0;
bool honeypotTriggered = false;
bool sounds = true;
int buzzer_pin = 13;

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
long old_seconds = 0;
long serial_info_seconds = 0;
int moveX = 0;
IPAddress currentIP;

const int flashButtonPin = 0;  // GPIO0 is connected to the flash button

WiFiManager wifiManager;

bool useWifiManager = true;
const char* ssid = "";
const char* password = "";

bool shouldSaveConfig = false;
bool useButtonToResetFlash = true;
bool hasControlsButtons = false;
bool debug = true;
bool headless = true;

bool securityScanActive = true;
bool skipFTPScan = true;
int vulnerabilitiesFound = 0;



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
  uint8_t encryptionType;
  int32_t rssi;
  uint8_t bssid[6];
  int32_t channel;
} NetworkInfo;

NetworkInfo knownNetworks[MAX_NETWORKS];
int numKnownNetworks = 0;
bool evilTwinDetected = false;

//wrapper functions for display 
void displayPrintln(String line = "") { display.println(line); }
void displaySetCursor(int x, int y) { display.setCursor(x, y); }
void displayPrint(String line) { display.print(line); }
void displayClearDisplay() { display.clearDisplay(); }
void displaySetSize(int size) { display.setTextSize(size); }
void displaySetTextColor(int color) { display.setTextColor(color); }
void displayPrintDate(const char* format, int day, int month, int year) { display.printf(format, day, month, year); }
void displayDisplay() { display.display(); }
void SerialPrintLn(String message) { if (debug) Serial.println(message); }
void SerialPrintLn(int message) { if (debug) Serial.println(message); }

void setup() {
  Serial.begin(115200);

  displayInit();
  netgotchiIntro();

  if (useWifiManager) {
    displayPrintln("TO Configure WIFI");
    displayPrintln("USE: AutoConnectAP");
  } else displayPrintln("Connecting to WiFi");

  displayDisplay();

  if (useWifiManager) {
    if (wifiManager.autoConnect("AutoConnectAP")) {
      displayPrintln("Connection Successful");
      displayDisplay();
    }
  } else {
    WiFi.begin(ssid, password);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    displayPrint(".");
    displayDisplay();
  }
  currentIP = WiFi.localIP();
  SerialPrintLn(currentIP.toString().c_str());
  timeClient.begin();
  ftpSrv.begin("admin", "admin");  // Set FTP username and password

  if (useButtonToResetFlash) pinMode(flashButtonPin, INPUT_PULLUP);

  saveCurrentNetworkInfos();
  initStars();
  buttonsInit();
}

void loop() {
  unsigned long currentMillis = millis();
  seconds = currentMillis / 1000;

  //display carousel
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (currentScreen == 0) {};
    if (currentScreen == 1) displayIPS();
    if (currentScreen == 2) NetworkStats();
    if (currentScreen > 2) {
      currentScreen = 0;
      animation++;
    }
    currentScreen++;
  }

  //ping scan 
  if (currentMillis - previousMillisScan >= intervalScan) {
    previousMillisScan = currentMillis;
    startScan = !startScan;
  }

  //network integrity
  if (currentMillis - previousMillisPing >= intervalPing) {
    previousMillisPing = currentMillis;
    scanOnce = true;
  }

  //sounds alert
  if (currentMillis - previousMillisSoundAlert >= intervalSound) {
    previousMillisSoundAlert = currentMillis;
    if (sounds && honeypotTriggered) playAlert();
  }

  //  Evil Twin scans
  if (currentMillis - previouslastEvilTwinCheck >= evilTwinScanInterval) {
    previouslastEvilTwinCheck = currentMillis;
    bool previousEvilTwinStatus = evilTwinDetected;
    evilTwinDetected = detectEvilTwin();
    if (evilTwinDetected && !previousEvilTwinStatus) {
      playAlert();
    }
  }

  //Ping Scan 
  if (startScan) {
    if (i < 256) {
      pingNetwork(i);
      i++;
    } else {
      i = 0;
      ipnum = 0;
      vulnerabilitiesFound = 0;
    }
  }

  //honeypot Checks
  ftpHoneypotScan();

  //animations loop 
  if (animation == 0) drawSpace();
  if (animation == 1) netgotchi_face();
  if (animation > 1) animation = 0;

  //button loops
  if (useButtonToResetFlash) buttonLoops();
  if (hasControlsButtons) controlsButtonLoop();
  //headless infos
  if (headless) headlessInfo();

  delay(5);

}


void headlessInfo() {
  if (seconds - serial_info_seconds > 1) {
    serial_info_seconds = seconds;
    SerialPrintLn(netgotchiCurrentFace + " Honeypot:" + (honeypotTriggered ? "breached" : "OK") + 
                  " EvilTwin:" + (evilTwinDetected ? "detected" : "OK") +
                  " Host-Found:" + String(ipnum) + " Vulnerabilities:" + String(vulnerabilitiesFound));
  }
}




