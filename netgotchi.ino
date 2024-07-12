// Netgotchi - lives to protect your network!
// Created by MXZZ https://github.com/MXZZ
// GNU General Public License v3.0

#include <ESP8266WiFi.h>
#include <ESPping.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiManager.h>  // Include the WiFiManager library
#include <ESP8266FtpServer.h>
#include <ESP8266mDNS.h>



#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
const float VERSION = 0.9;


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
FtpServer ftpSrv;  // Create an instance of the FTP server


const int NUM_STARS = 100;
float stars[NUM_STARS][3];
float ufoX = SCREEN_WIDTH / 2;
float ufoY = SCREEN_HEIGHT / 2;
float ufoZ = 0;

String status = "Idle";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

unsigned long previousMillis = 0;
unsigned long previousMillisScan = 0;
unsigned long previousMillisPing = 0;



const long interval = 20000;  //
int i = 0;
int ipnum = 0;   // display counter
int iprows = 0;  // ip rows
int currentScreen = 0;
int max_ip = 255;
bool startScan = false;
const long intervalScan = 60000 * 4;
const long intervalPing = 60000 * 5;

int seconds = 0;


int ips[255] = {};

bool pingScanDetected = false;
unsigned long lastPingTime = 0;
bool honeypotTriggered = false;

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

const int flashButtonPin = 0;  // GPIO0 is connected to the flash button

WiFiManager wifiManager;


//Use wifi manager  or use the SSID/PASSWORD credential below
bool useWifiManager = true;
//ssid and password are used only when useWifiManager == false
const char* ssid = "";
const char* password = "";

bool shouldSaveConfig = false;
bool useButtonToResetFlash = true;
bool debug = true;
bool headless = true;

bool securityScanActive = true;
bool skipFTPScan = true;
int vulnerabilitiesFound=0;

// List of potentially dangerous services and their ports
struct Service {
  const char* name;
  uint16_t port;
};
//Scan for dangerous services opened in the network
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

//Wrapper functions for other display compatibility
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

void displayPrintDate(const char* format, int day, int month, int year) {
  display.printf(format, day, month, year);
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
///end of wrapper functions


void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    SerialPrintLn(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  displayClearDisplay();
  displaySetSize(1);
  displaySetTextColor(1);  //white color
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

    if (wifiManager.autoConnect("AutoConnectAP")) {
      displayPrintln("Connection Successful");
      displayDisplay();
    }
  } else {
    WiFi.begin(ssid, password);
  }

  //WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    displayPrint(".");
    displayDisplay();
  }
  currentIP = WiFi.localIP();
  SerialPrintLn(currentIP.toString().c_str());
  timeClient.begin();
  initStars();
  ftpSrv.begin("admin", "admin");  // Set FTP username and password

  // Initialize the flash button pin as input
  if (useButtonToResetFlash) pinMode(flashButtonPin, INPUT_PULLUP);
}

void loop() {
  unsigned long currentMillis = millis();
  seconds = currentMillis / 1000;

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


  if (currentMillis - previousMillisScan >= intervalScan) {
    previousMillisScan = currentMillis;
    startScan = !startScan;
  }

  if (currentMillis - previousMillisPing >= intervalPing) {
    previousMillisPing = currentMillis;
    scanOnce = true;
  }


  if (startScan) {
    if (i < 256) {
      pingNetwork(i);
      i++;
    } else {
      i = 0;
      ipnum = 0;
      vulnerabilitiesFound=0;
    }
  }

  ftpHoneypotScan();

  if (animation == 0) drawSpace();
  if (animation == 1) netgotchi_face();
  if (animation > 1) animation = 0;

  if (useButtonToResetFlash) buttonLoops();
  if (headless) headlessInfo();

  delay(5);
}

void NetworkStats() {
  displayClearDisplay();
  displaySetCursor(0, 8);
  if (WiFi.status() == WL_CONNECTED) networkStatus = "connected";
  else networkStatus = "disconnected";
  displayPrint("Network: " + networkStatus);
  displaySetCursor(0, 16);

  if (scanOnce) {
    IPAddress ip(1, 1, 1, 1);  // ping goole cloudflare
    SerialPrintLn("pinging cloudflare");

    if (Ping.ping(ip, 2)) {
      externalNetworkStatus = "Reachable";
      displayPrintln();
      scanOnce = false;
      stats = "\n min: " + String(Ping.minTime()) + "ms \n avg: " + String(Ping.averageTime()) + "ms \n max: " + String(Ping.maxTime()) + "ms";
      delay(500);
      SerialPrintLn("ping sent");
      SerialPrintLn(stats);

      //serviceDiscover();

    } else externalNetworkStatus = "Unreachable";
  }
  displayPrintln("Network Speed: " + stats);
  displayPrintln("Internet: " + externalNetworkStatus);
  displayDisplay();
  delay(5000);
}

void ftpHoneypotScan() {
  ftpSrv.handleFTP();
  // Check for FTP connections
  if (ftpSrv.returnHoneypotStatus()) {
    honeypotTriggered = true;
  }
}

void drawSpace() {

  displayClearDisplay();
  updateAndDrawStars();
  drawUFO();
  displayTimeAndDate();
  displayDisplay();
  delay(10);
}

void initStars() {
  for (int i = 0; i < NUM_STARS; i++) {
    stars[i][0] = random(-1000, 1000);
    stars[i][1] = random(-1000, 1000);
    stars[i][2] = random(1, 1000);
  }
}

void updateAndDrawStars() {
  for (int i = 0; i < NUM_STARS; i++) {
    stars[i][2] -= 5;
    if (stars[i][2] <= 0) stars[i][2] = 1000;

    int x = (stars[i][0] / stars[i][2]) * 64 + SCREEN_WIDTH / 2;
    int y = (stars[i][1] / stars[i][2]) * 32 + SCREEN_HEIGHT / 2;

    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
      display.drawPixel(x, y, WHITE);
    }
  }
}

void drawUFO() {
  int ufoSize = 8;
  display.drawLine(ufoX - ufoSize, ufoY, ufoX + ufoSize, ufoY, WHITE);
  display.drawLine(ufoX, ufoY - ufoSize / 2, ufoX, ufoY + ufoSize / 2, WHITE);
  display.drawCircle(ufoX, ufoY, ufoSize / 2, WHITE);

  ufoX = SCREEN_WIDTH / 2 + sin(millis() / 1000.0) * 20;
  ufoY = SCREEN_HEIGHT / 2 + cos(millis() / 1500.0) * 10;
}

void displayTimeAndDate() {
  timeClient.update();
  String formattedTime = timeClient.getFormattedTime();
  time_t epochTime = timeClient.getEpochTime();
  struct tm* ptm = gmtime((time_t*)&epochTime);

  int currentDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon + 1;
  int currentYear = ptm->tm_year + 1900;

  displaySetSize(1);
  displaySetTextColor(WHITE);
  displaySetCursor(5, 0);
  displayPrint(formattedTime);
  displaySetCursor(0, 8);
  displayPrintDate("%02d/%02d/%d", currentDay, currentMonth, currentYear);
  displaySetCursor(0, 55);
  displayPrint("Hosts:" + String(ipnum) + " VU:"+ String(vulnerabilitiesFound));
  displaySetCursor(75, 0);
  displayPrint("Honeypot");
  if (honeypotTriggered) {
    if (((seconds % 2) == 0)) {
      displaySetCursor(80, 8);
      displayPrint("Breached");
      displaySetCursor(40, 16);
      displayPrint(ftpSrv.getHoneyPotBreachIPandTime());
    }
  } else {
    displaySetCursor(80, 8);
    displayPrint("OK");
  }

  displaySetCursor(90, 55);
  if (startScan)
    displayPrint("Scan");
  else displayPrint("Idle");
}


void displayIPS() {
  displayClearDisplay();
  displaySetCursor(0, 0);
  displayPrintln("Found Hosts:");

  //Ipprefix is based on the current assigned IP type
  //change this to hardcode your subnet
  String ipprefix = String(currentIP[0]) + "." + String(currentIP[1]) + "." + String(currentIP[2]) + ".";

  for (int j = 0; j < max_ip; j++) {

    if (ips[j] == 1 || ips[j] == -1 || ips[j] == 2) {
      if (iprows >= 4) {
        displayClearDisplay();
        displaySetCursor(5, 0);
        displayPrintln("Hosts:" + String(ipnum));
        iprows = 0;
      }
      displaySetCursor(0, 20 + (iprows)*10);
      if (ips[j] == 1) {
        String al = ipprefix + String(j) + " UP";
        displayPrintln(al);
        iprows++;
      }
      if (ips[j] == 2) {
        String al = ipprefix + String(j) + " WRNG!";
        displayPrintln(al);
        iprows++;
      }
      if (ips[j] == -1) {
        String dc = ipprefix + String(j) + " DOWN";
        displayPrintln(dc);
        iprows++;
      }

      delay(500);  // Small delay to avoid overwhelming the display

      if (iprows == 5) delay(3000);
      displayDisplay();
    }
  }
  if (ipnum > 0) delay(5000);
}

void pingNetwork(int i) {
  status = "Scanning";
  //change this to hardcode your subnet
  IPAddress ip = IPAddress(currentIP[0], currentIP[1], currentIP[2], i);
  if (Ping.ping(ip, 1)) {
    SerialPrintLn("Alive");
    SerialPrintLn(ip.toString().c_str());

    iprows++;
    ipnum++;
    //store
    ips[i] = 1;

    if (securityScanActive) {
      int scanresult = scanForDangerousServices(ip);
      if (scanresult == 1) {
        SerialPrintLn(String(scanresult));
        ips[i] = 2;
      }
    }
  } else {
    //not found
    if (ips[i] == -1) ips[i] = 0;       //disconnected - remove
    else if (ips[i] == 1) ips[i] = -1;  //recently disconnected
    else if (ips[i] == 2) ips[i] = -1;  // device disconnected, with warning
    else ips[i] = 0;
  }
}

void netgotchi_face() {
  displayClearDisplay();
  updateAndDrawStars();  // Draw the star effect
  displayTimeAndDate();
  displaySetSize(2);
  drawnetgotchiFace(animState);

  if (seconds - old_seconds > 1) {
    moveX = moveX + random(-5, 5);
    if (moveX > 20) moveX = 5;
    if (moveX < -20) moveX = -5;

    old_seconds = seconds;
    animState++;
    if (animState > 5) animState = 0;
  }
  displayDisplay();
  displaySetSize(1);
}

void drawnetgotchiFace(int state) {
  displaySetCursor(30 + moveX, 30);
  if (honeypotTriggered == false) {
    if (state == 0) {
      netgotchiCurrentFace = netgotchiFace;
    }
    if (state == 1) {
      netgotchiCurrentFace = netgotchiFace2;
    }

    if (state == 2) {
      netgotchiCurrentFace = netgotchiFaceBlink;
    }

    if (state == 3) {
      netgotchiCurrentFace = netgotchiFaceSleep;
    }
    if (state == 4) {
      netgotchiCurrentFace = netgotchiFaceSurprised;
    }
    if (state == 5) {
      netgotchiCurrentFace = netgotchiFaceHappy;
    }
  } else {
    if (state == 0) {
      netgotchiCurrentFace = netgotchiFaceSad;
    }
    if (state == 1) {
      netgotchiCurrentFace = netgotchiFaceSad2;
    }
    if (state == 2) {
      netgotchiCurrentFace = netgotchiFaceSuspicious;
    }
    if (state == 3) {
      netgotchiCurrentFace = netgotchiFaceSuspicious2;
    }
    if (state == 4) {
      netgotchiCurrentFace = netgotchiFaceHit;
    }
    if (state == 5) {
      netgotchiCurrentFace = netgotchiFaceHit2;
    }
  }
  displayPrintln(netgotchiCurrentFace);
}


void buttonLoops() {
  // Check if the flash button is pressed
  if (digitalRead(flashButtonPin) == LOW) {
    // Debounce delay
    delay(50);
    if (digitalRead(flashButtonPin) == LOW) {
      // Button is still pressed, proceed to erase EEPROM and WiFiManager settings
      displayClearDisplay();
      displayPrintln("Flash button pressed. WiFiManager settings...");
      // Erase WiFiManager settings
      wifiManager.resetSettings();

      displayPrintln("EEPROM and WiFiManager settings erased.");
      displayPrintln("Restart this device");

      // Optional: Add a delay to prevent multiple erases in quick succession
      delay(10000);
      ESP.restart();
    }
  }
}

void headlessInfo() {
  //slow down the print in the serial console
  if (seconds - serial_info_seconds > 1) {
    serial_info_seconds = seconds;
    SerialPrintLn(netgotchiCurrentFace + " Honeypot :" + (honeypotTriggered ? "breached" : "OK") + (" Host-Found:" + String(ipnum)) + (" Vulnerabilities:" +  String(vulnerabilitiesFound)));
  }
}

int scanForDangerousServices(IPAddress ip) {
  WiFiClient client;
  for (int i = 0; i < sizeof(dangerousServices) / sizeof(dangerousServices[0]); ++i) {
    //skip FTP scan if you have other netgotchi / ftp server in the network.
    if(skipFTPScan && dangerousServices[i].name == "FTP") continue;
    if (client.connect(ip, dangerousServices[i].port)) {
      Serial.print("Open port found: ");
      Serial.print(dangerousServices[i].name);
      Serial.print(" (");
      Serial.print(dangerousServices[i].port);
      Serial.print(") on ");
      Serial.println(ip);
      client.stop();
      vulnerabilitiesFound++;
      return 1;
    }
  }
  return 0;
}
