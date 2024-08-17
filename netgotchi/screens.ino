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
  delay(1);
}

void displayRippleSpace() {
  displayClearDisplay();
  drawRipple();
  netgotchi_face();

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
  delay(1);
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


int frame = 0;
int numCircles = 10;
int maxRadius = 70;
void drawRipple()
{
  int radius = (frame  * 10) % maxRadius;
  displayDrawCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, radius, 1);
  frame++;
  delay(100);
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

  if(ipnum>0) {
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
        if (iprows == 4) delay(1000);
        displayDisplay();
      }
    }
  }
  else nextScreen();
}

void displayNetgotchi()
{
  displayClearDisplay();
  updateAndDrawStars();

  if(enableNetworkMode)
  {
    displayTimeAndDate();
    displayStats();
  }
  else displayOfflineMode();

  netgotchi_face();
  displayDisplay();

}

void netgotchi_face() {
  
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

void displaySettings()
{
  displayClearDisplay();
  displaySetCursor(0, 0);
  displayPrintln("Settings v." + String(VERSION));
  displaySetCursor(0, 10);

  for(int i=0; i< settingLength ;i++)
  {
    if(selectedSetting == i)
    displayPrintln(">"+settings[i]);
    else
    displayPrintln(" "+settings[i]);
  }
  displayDisplay();
}

void displayNetgotchiStats(){
  displayClearDisplay();
  displaySetCursor(0, 0);
  displayPrintln("Netgotchi v." + String(VERSION));
  displaySetCursor(0, 10);

  displayPrintln("IP:  " + currentIP.toString() );
  displayPrintln("Uptime:" + String(seconds)+"sec");
  if( WiFi.status() == WL_CONNECTED)displayPrintln("SSID:" + WiFi.SSID());

  
  displayDisplay();
}

void screenAnimations()
{
  //animations loop on the same carousel page
  
  if (animation == 0) displayNetgotchi();
  if (animation == 1) drawSpace();
  if (animation > max_anim) animation = 0;
}


int getPixelAt(int x, int y) {
    // Check if coordinates are within bounds
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
        return -1;  // Out of bounds
    }

    // Calculate the index of the byte in the buffer
    int byteIndex = (y / 8) * SCREEN_WIDTH + x;  // Find which byte
    int bitIndex = y % 8;  // Find which bit within the byte

    // Get the byte value from the buffer
    uint8_t* buffer = displayGetBuffer();
    uint8_t byteValue = buffer[byteIndex];
  
    // Check if the pixel is set (1 for white, 0 for black)
    bool isWhite = (byteValue & (1 << bitIndex)) != 0;
    return isWhite ? 1 : 0;
}

String getPixelMatrix() {
    String matrix = "[";
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        if (y > 0) matrix += ",";
        matrix += "[";
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            if (x > 0) matrix += ",";
            matrix += getPixelAt(x, y);
        }
        matrix += "]";
    }
    matrix += "]";
    return matrix;
}

void nextScreen(){
  currentScreen++;
  if (currentScreen > maxScreens) {
    currentScreen = 0;
    //change animation
    animation++;
    if(animation>max_anim)animation=0;
  }
}
