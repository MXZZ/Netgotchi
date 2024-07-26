#include <WiFi.h>
#include <ESPping.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiManager.h>
#include <ESP32FtpServer.h>
#include <ESPmDNS.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
const float VERSION = 1.0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
FtpServer ftpSrv;

const int NUM_STARS = 100;
float stars[NUM_STARS][3];
float ufoX = SCREEN_WIDTH / 2;
float ufoY = SCREEN_HEIGHT / 2;
float ufoZ = 0;
long timeOffset = 7200;

String status = "Idle";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", timeOffset);

unsigned long previousMillis = 0;
unsigned long previousMillisScan = 0;
unsigned long previousMillisPing = 0;
unsigned long previousMillisSoundAlert = 0;

const long interval = 20000;
int i = 0;
int ipnum = 0;
int iprows = 0;
int currentScreen = 0;
int max_ip = 255;
bool startScan = false;
const long intervalScan = 60000 * 4;
const long intervalPing = 60000 * 5;
const long intervalSound = 60000 * 2;

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
int animState = 0;
int animation = 0;
long old_seconds = 0;
long serial_info_seconds = 0;
int moveX = 0;
IPAddress currentIP;

const int flashButtonPin = 0;

WiFiManager wifiManager;

bool useWifiManager = true;
const char* ssid = "";
const char* password = "";

bool shouldSaveConfig = false;
bool useButtonToResetFlash = true;
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

void displayPrintln(String line = "") {
  display.println(line);
}

void displaySetCursor(int x, int y) {
  display.setCursor(x, y);
}

void displayPrint(String line) {
  display.print(line);
}

void displayClearDisplay() {
  display.clearDisplay();
}

void displaySetSize(int size) {
  display.setTextSize(size);
}

void displaySetTextColor(int color) {
  display.setTextColor(color);
}

void displayPrintDate(const char* format, const char* date) {
  display.printf(format, date);
}

void displayDisplay() {
  display.display();
}

void SerialPrintLn(String message) {
  if (debug) Serial.println(message);
}
void SerialPrintLn(int message) {
  if (debug) Serial.println(message);
}

void setup() {
  Serial.begin(115200);

  SerialPrintLn("Initializing display...");
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C) && !headless) {
    SerialPrintLn(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  displayClearDisplay();
  displaySetSize(1);
  displaySetTextColor(1);
  displaySetCursor(0, 0);
  displayPrintln("Netgotchi v." + String(VERSION));
  displayPrintln("created by MXZZ ");
  delay(1000);

  if (useWifiManager) {
    displayPrintln("TO Configure WIFI");
    displayPrintln("USE: AutoConnectAP");
  } else displayPrintln("Connecting to WiFi");

  displayDisplay();

  if (useWifiManager) {
    SerialPrintLn("Starting WiFi Manager...");
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
  SerialPrintLn("Connected to WiFi. IP address: " + currentIP.toString());
  timeClient.begin();
  initStars();
  ftpSrv.begin("admin", "admin");

  if (useButtonToResetFlash) pinMode(flashButtonPin, INPUT_PULLUP);
}

void loop() {
  unsigned long currentMillis = millis();
  seconds = currentMillis / 1000;

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (currentScreen == 0) {};
    if (currentScreen == 1) displayIPAddresses();
    if (currentScreen == 2) NetworkStats();
    if (currentScreen > 2) {
      currentScreen = 0;
      animation++;
    }
    currentScreen++;
    SerialPrintLn("Screen updated to: " + String(currentScreen));
  }

  if (currentMillis - previousMillisScan >= intervalScan) {
    previousMillisScan = currentMillis;
    startScan = !startScan;
    SerialPrintLn("Start scan: " + String(startScan));
  }

  if (currentMillis - previousMillisPing >= intervalPing) {
    previousMillisPing = currentMillis;
    scanOnce = true;
    SerialPrintLn("Set scanOnce to true");
  }

  if (currentMillis - previousMillisSoundAlert >= intervalSound) {
    previousMillisSoundAlert = currentMillis;
    if (sounds) playAlert();
    SerialPrintLn("Playing sound alert");
  }

  if (headless) {
    headlessInfo();
  } else {
    buttonLoops();
  }

  if (startScan) {
    for (i = 1; i < max_ip; i++) {
      pingNetwork(i);
    }
    SerialPrintLn("Completed ping network scan.");
  }

  if (scanOnce) {
    scanOnce = false;
    ftpHoneypotScan();
    SerialPrintLn("Completed FTP honeypot scan.");
  }

  if (!headless) {
    switch (animation) {
      case 0: netgotchi_face(); break;
      case 1: drawSpace(); break;
      default: displayPrintln("Error in Animation"); break;
    }
  }
}

void displayIPAddresses() {
  displayClearDisplay();
  displaySetCursor(0, 0);
  displaySetSize(1);
  displayPrintln("Connected IPs:");
  for (int i = 0; i < ipnum; i++) {
    displayPrintln(String(ips[i]));
  }
  displayDisplay();
  SerialPrintLn("Displayed IP addresses.");
}

void NetworkStats() {
  if (Ping.ping(currentIP)) {
    externalNetworkStatus = "Connected";
    stats = "\n min: " + String(Ping.minTime()) + "ms\n avg: " + String(Ping.averageTime()) + "ms\n max: " + String(Ping.maxTime()) + "ms";
  } else {
    externalNetworkStatus = "Unreachable";
  }
  displayPrintln("External Network: " + externalNetworkStatus);
  displayPrintln(stats);
  displaySetCursor(0, 32);
  displayPrintln("Network security scan");

  if (pingScanDetected) {
    displayPrintln("ALERT! scan detected");
    displayPrintln("Last scan: " + String(lastPingTime) + "sec");
    SerialPrintLn("Ping scan detected.");
  } else {
    displayPrintln("Nothing suspicious found");
  }
  displayDisplay();
}

void netgotchi_face() {
  displayClearDisplay();
  displaySetSize(2);
  displaySetCursor(40, 40);
  if (WiFi.status() == WL_CONNECTED) {
    displayPrint(netgotchiFaceHappy);
    netgotchiCurrentFace = netgotchiFaceHappy;
  } else {
    displayPrint(netgotchiFaceSad);
    netgotchiCurrentFace = netgotchiFaceSad;
  }
  displaySetSize(1);
  displaySetCursor(0, 0);
  displayPrint("Netgotchi");
  displaySetCursor(90, 1);
  String formattedDate = timeClient.getFormattedTime();
  displayPrintDate("%s", formattedDate.c_str());
  displayDisplay();
  SerialPrintLn("Displayed netgotchi face: " + netgotchiCurrentFace);
}

void drawSpace() {
  displayClearDisplay();
  for (int i = 0; i < NUM_STARS; i++) {
    stars[i][2] -= 0.5;
    if (stars[i][2] <= 0) {
      stars[i][0] = random(-25, 25);
      stars[i][1] = random(-25, 25);
      stars[i][2] = 50;
    }

    int x = int(stars[i][0] / stars[i][2] * 100 + SCREEN_WIDTH / 2);
    int y = int(stars[i][1] / stars[i][2] * 100 + SCREEN_HEIGHT / 2);
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) display.drawPixel(x, y, SSD1306_WHITE);
  }
  display.setCursor(int(ufoX), int(ufoY));
  if (animState == 0) {
    displayPrint(netgotchiInUfo1);
    animState = 1;
  } else {
    displayPrint(netgotchiInUfo2);
    animState = 0;
  }
  display.display();
  SerialPrintLn("Displayed space animation.");
}

void pingNetwork(int i) {
  IPAddress ip(192, 168, 1, i);
  if (Ping.ping(ip)) {
    if (ips[ipnum] != i) {
      ips[ipnum] = i;
      ipnum++;
      lastPingTime = seconds;
      pingScanDetected = true;
      SerialPrintLn("Pinged IP: " + ip.toString());
    }
  }
}

void initStars() {
  for (int i = 0; i < NUM_STARS; i++) {
    stars[i][0] = random(-25, 25);
    stars[i][1] = random(-25, 25);
    stars[i][2] = random(1, 50);
  }
  SerialPrintLn("Initialized stars for space animation.");
}

void ftpHoneypotScan() {
  ftpSrv.handleFTP(); 
  // we need to make this better
  if (!skipFTPScan) { 
    honeypotTriggered = true;
    SerialPrintLn("Honeypot triggered!");
    vulnerabilitiesFound++;
  }
}

void buttonLoops() {
  int buttonState = digitalRead(flashButtonPin);
  if (buttonState == LOW) {
    displayPrintln("Resetting WiFi config...");
    displayDisplay();
    wifiManager.resetSettings();
    delay(2000);  
    ESP.restart(); 
  }
}

void headlessInfo() {
  if (old_seconds + 10 < seconds) {
    old_seconds = seconds;
    if (WiFi.status() == WL_CONNECTED) netgotchiCurrentFace = netgotchiFaceHappy;
    else netgotchiCurrentFace = netgotchiFaceSad;
    SerialPrintLn("Running in headless mode...");
    SerialPrintLn("Status: " + netgotchiCurrentFace);
  }
}

void playAlert() {
  tone(buzzer_pin, 1000, 500); 
  delay(1000);
  noTone(buzzer_pin);
  SerialPrintLn("Played sound alert.");
}
