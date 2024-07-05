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

FtpServer ftpSrv;  // Create an instance of the FTP server


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


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
String stats = "not avaiable";
String pwnagotchiFace = "(-v_v)";
String pwnagotchiFace2 = "(v_v-)";
String pwnagotchiFaceBlink = "( .__.)";
String pwnagotchiFaceSleep = "(-__- )";
String pwnagotchiFaceSurprised = "(o__o)";
String pwnagotchiFaceHappy = "(^=^)";
String pwnagotchiFaceSad = "(T_T)";
String pwnagotchiFaceSad2 = "(T__T)";
String pwnagotchiFaceSuspicious = "(>_>)";
String pwnagotchiFaceSuspicious2 = "(<_<)";
String pwnagotchiFaceHit = "(x_x)";
String pwnagotchiFaceHit2 = "(X__X)";
String pwnagotchiFaceStarryEyed = "(*_*)";
String pwnagotchiScreenMessage = "Saving planets!";
int animState = 0;
int animation = 0;
long old_seconds = 0;
int moveX = 0;
String currentIP = "";


//**
//Type of Subnet supported
//192.168.0.0/24 = type 0
//192.168.1.0/24 = type 1
//192.168.88.0/24 = type 2
//192.168.100.0/24  = type 3
// or add your own subnet in the pingNetwork Function
int subnet = 0;


//Use wifi manager  or use the SSID/PASSWORD credential below
bool useWifiManager = true;
//ssid and password are used only when useWifiManager == false
const char* ssid = "";
const char* password = "";


void setup() {
  Serial.begin(115200);

  WiFiManager wifiManager;

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Connecting to WiFi");

  if (useWifiManager) {
    if (wifiManager.autoConnect("AutoConnectAP")) {
      display.println("Connection Successful");
      display.display();

    } else {
      display.println("Select Wifi AutoConnectAP");
      display.println("to run Wifi Setup");
      display.display();
    }
  } else {
    WiFi.begin(ssid, password);
  }

  //WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
  }
  currentIP = WiFi.localIP().toString().c_str();
  Serial.println(currentIP);
  timeClient.begin();
  initStars();
  ftpSrv.begin("admin", "admin");  // Set FTP username and password
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
    if (i < 255) {
      pingNetwork(i);
      i++;
    } else {
      i = 0;
      ipnum = 0;
    }
  }


  ftpHoneypotScan();

  if (animation == 0) drawSpace();
  if (animation == 1) pwnagotchi_face();
  if (animation > 1) animation = 0;

  delay(5);
}

void NetworkStats() {
  display.clearDisplay();
  display.setCursor(0, 8);
  if (WiFi.status() == WL_CONNECTED) networkStatus = "connected";
  else networkStatus = "disconnected";
  display.print("Network: " + networkStatus);
  display.setCursor(0, 16);

  if (scanOnce) {
    IPAddress ip(1, 1, 1, 1);  // ping goole cloudflare
    Serial.println("pinging cloudflare");

    if (Ping.ping(ip, 2)) {
      externalNetworkStatus = "Reachable";
      display.println();
      scanOnce = false;
      stats = "\n min: " + String(Ping.minTime()) + "ms \n avg: " + String(Ping.averageTime()) + "ms \n max: " + String(Ping.maxTime()) + "ms";
      delay(500);
      Serial.println("ping sent");
      Serial.println(stats);

      //serviceDiscover();

    } else externalNetworkStatus = "Unreachable";
  }
  display.println("Network Speed: " + stats);
  display.println("Internet: " + externalNetworkStatus);
  display.display();
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

  display.clearDisplay();
  updateAndDrawStars();
  drawUFO();
  displayTimeAndDate();
  display.display();
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

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(5, 0);
  display.print(formattedTime);
  display.setCursor(0, 8);
  display.printf("%02d/%02d/%d", currentDay, currentMonth, currentYear);
  display.setCursor(0, 55);
  display.print("Host found:" + String(ipnum));
  display.setCursor(75, 0);
  display.print("Honeypot");
  if (honeypotTriggered) {
    if (((seconds % 2) == 0)) {
      display.setCursor(80, 8);
      display.print("Breached");
    }
  } else {
    display.setCursor(80, 8);
    display.print("OK");
  }

  display.setCursor(90, 55);
  if (startScan)
    display.print("Scan");
  else display.print("Idle");
}

char hostString[16] = { 0 };
void serviceDiscover() {
  if (!MDNS.begin(hostString)) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    //    Serial.println("mDNS responder started");
    //    MDNS.addService("esp", "tcp", 8080);  // Announce esp tcp service on port 8080

    Serial.println("Sending mDNS query");
    int n = MDNS.queryService("https", "tcp");  // Send out query for esp tcp services
    Serial.println("mDNS query done");
    if (n == 0) {
      Serial.println("no services found");
    } else {
      Serial.print(n);
      Serial.println(" service(s) found");
      for (int i = 0; i < n; ++i) {
        // Print details for each service found
        Serial.print(MDNS.hostname(i));
        Serial.print(MDNS.IP(i));
        // Serial.print(MDNS.port(i));
      }
    }
  }
}

void displayIPS() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Found Hosts:" + String(ipnum));
  display.println("This:"+ currentIP);

  //Ipprefix is based on the subnet type
  String ipprefix = "";

  //192.168.0.0/24 = type 0
  //192.168.1.0/24 = type 1
  //192.168.88.0/24 = type 2
  //192.168.100.0/24  = type 3

  if (subnet == 0) ipprefix = "192.168.0.";
  if (subnet == 1) ipprefix = "192.168.1.";
  if (subnet == 2) ipprefix = "192.168.88.";
  if (subnet == 3) ipprefix = "192.168.100.";

  for (int j = 0; j < max_ip; j++) {

    if (ips[j] == 1 || ips[j] == -1) {
      if (iprows >= 4) {
        display.clearDisplay();
        display.setCursor(5, 0);
        display.println("Hosts:" + String(ipnum));

        iprows = 0;
      }
      display.setCursor(0, 20 + (iprows)*10);
      if (ips[j] == 1) {
        String al = ipprefix + String(j) + " alive";
        display.println(al);
        iprows++;
      }
      if (ips[j] == -1) {
        String dc = ipprefix + String(j) + " disconnected";
        display.println(dc);
        iprows++;
      }
      delay(1500);  // Small delay to avoid overwhelming the display
      display.display();
    }
  }
  if (ipnum > 0) delay(5000);
}

void pingNetwork(int i) {
  status = "Scanning";
  IPAddress ip(192, 168, 0, i);
  if (subnet == 0) IPAddress ip(192, 168, 0, i);  // Change to your network's IP range
  if (subnet == 1) IPAddress ip(192, 168, 1, i);
  if (subnet == 2) IPAddress ip(192, 168, 88, i);
  if (subnet == 3) IPAddress ip(192, 168, 100, i);
  if (Ping.ping(ip, 1)) {
    iprows++;
    ipnum++;
    //store
    ips[i] = 1;
  } else {
    //not found
    if (ips[i] == -1) ips[i] = 0;       //disconnected - remove
    else if (ips[i] == 1) ips[i] = -1;  //recently disconnected
    else ips[i] = 0;
  }
}

void pwnagotchi_face() {
  display.clearDisplay();
  updateAndDrawStars(); // Draw the star effect
  displayTimeAndDate();
  display.setTextSize(2);
  drawPwnagotchiFace(animState);

  if (seconds - old_seconds > 1) {
    moveX = moveX + random(-5, 5);
    if (moveX > 20) moveX = 5;
    if (moveX < -20) moveX = -5;

    old_seconds = seconds;
    animState++;
    if (animState > 5) animState = 0;
  }
  display.display();
  display.setTextSize(1);
}

void drawPwnagotchiFace(int state) {
  display.setCursor(30 + moveX, 30);

  if (honeypotTriggered == false) {
    if (state == 0) {
      display.println((pwnagotchiFace));
    }
    if (state == 1) {
      display.println((pwnagotchiFace2));
    }

    if (state == 2) {
      display.println((pwnagotchiFaceBlink));
    }

    if (state == 3) {
      display.println((pwnagotchiFaceSleep));
    }
    if (state == 4) {
      display.println((pwnagotchiFaceSurprised));
    }
    if (state == 5) {
      display.println((pwnagotchiFaceHappy));
    }
  } else {
    if (state == 0) {
      display.println((pwnagotchiFaceSad));
    }
    if (state == 1) {
      display.println((pwnagotchiFaceSad2));
    }
    if (state == 2 ) {
      display.println((pwnagotchiFaceSuspicious));
    }
    if (state == 3 ) {
      display.println((pwnagotchiFaceSuspicious2));
    }
    if (state == 4 ) {
      display.println((pwnagotchiFaceHit));
    }
    if (state == 5 ) {
      display.println((pwnagotchiFaceHit2));
    }
  }
}
