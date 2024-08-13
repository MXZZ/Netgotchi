//Screens functions



void displayInit()
{
  //to skip if the board has not display 
  if(hasDisplay)
  {
    //display initializer
    if(oled_type_ssd1306){
      if (!display.begin(2, 0x3C)) { 
        // add "SSD1306_SWITCHCAPVCC, 0x3C" in the begin() if screen doesn't work. 
        SerialPrintLn("SSD1306 allocation failed");
        for (;;);
      }
    }
    else
    { 
      if (!display.begin()) { 
        SerialPrintLn("Display allocation failed");
        for (;;);
      }
    }
  }
}

void drawSpace() {
  displayClearDisplay();
  updateAndDrawStars();
  drawUFO();
  if(enableNetworkMode)
  { 
    displayTimeAndDate();
    displayStats();
  }
  else
  {
    displayOfflineMode();
  }
  displayDisplay();
  delay(10);
}

void NetworkStats() {
  displayClearDisplay();
  displaySetCursor(0, 8);
  if (WiFi.status() == WL_CONNECTED) networkStatus = "connected";
  else networkStatus = "disconnected";
  displayPrint("Network: " + networkStatus);
  displaySetCursor(0, 16);

  if (scanOnce) {
    IPAddress ip(1, 1, 1, 1);  // ping cloudflare
    SerialPrintLn("pinging cloudflare");

    if (Ping.ping(ip, 2)) {
      externalNetworkStatus = "Reachable";
      displayPrintln();
      scanOnce = false;
      stats = "\n min: " + String(Ping.minTime()) + "ms \n avg: " + String(Ping.averageTime()) + "ms \n max: " + String(Ping.maxTime()) + "ms";
      delay(500);
      SerialPrintLn("ping sent");
      SerialPrintLn(stats);
    } else externalNetworkStatus = "Unreachable";
  }
  displayPrintln("Network Speed: " + stats);
  displayPrintln("Internet: " + externalNetworkStatus);
  displayDisplay();
  delay(5000);
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
      displayDrawPixel(x, y, 1);
    }
  }
}

void drawUFO() {
  int ufoSize = 8;
  displayDrawLine(ufoX - ufoSize, ufoY, ufoX + ufoSize, ufoY, 1);
  displayDrawLine(ufoX, ufoY - ufoSize / 2, ufoX, ufoY + ufoSize / 2, 1);
  displayDrawCircle(ufoX, ufoY, ufoSize / 2, 1);

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
  displaySetTextColor(1);
  displaySetCursor(5, 0);
  displayPrint(formattedTime);
  displaySetCursor(0, 8);
  displayPrintDate("%02d/%02d/%d", currentDay, currentMonth, currentYear);
  
}

void displayStats()
{
  displaySetCursor(0, 55);
  displayPrint("Hosts:" + String(ipnum) + " VU:" + String(vulnerabilitiesFound));
  displaySetCursor(75, 0);
  displayPrint("Honeypot");
  if (honeypotTriggered) {
    if (((seconds % 2) == 0)) {
      displaySetCursor(80, 8);
      displayPrint("Breached");
      displaySetCursor(40, 16);

      #if defined(ESP8266)
        displayPrint(ftpSrv.getHoneyPotBreachIPandTime());
      #endif
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

      delay(500);
      if (iprows == 4) delay(3000);
      displayDisplay();
    }
  }
  if (ipnum > 0) delay(5000);
}

void netgotchi_face() {
  displayClearDisplay();
  updateAndDrawStars();

  if(enableNetworkMode)
  {
    displayTimeAndDate();
    displayStats();
  }
  else displayOfflineMode();
  
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


void displayOfflineMode()
{
  displaySetCursor(0, 55);
  displayPrint("Netgotchi is Offline");
}

void netgotchiIntro()
{
  displayClearDisplay();
  displaySetSize(1);
  displaySetTextColor(1);  //white color
  displaySetCursor(0, 0);
  displayPrintln("Netgotchi v." + String(VERSION));
  displayPrintln("created by MXZZ ");
  delay(500);
}