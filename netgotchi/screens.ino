// Screens functions — margini e testi ottimizzati per ST7789 320x172

// --- Se stai usando ST7789, la displayInit() è in netgotchi.ino ---
#ifdef USE_ST7789
// nothing here
#else
void displayInit() {
  if (hasDisplay) {
    if (oled_type_ssd1306) {
      if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        SerialPrintLn("SSD1306 allocation failed");
        for (;;);
      }
    } else {
      if (!display.begin()) {
        SerialPrintLn("Display allocation failed");
        for (;;);
      }
    }
  }
}
#endif

// ---------- Layout helpers ----------
static const int FONT_W = 6;         // font classico Adafruit_GFX
static const int FONT_H = 8;
static const int UI_MARGIN_X = 12;   // margine sinistro/destro
static const int UI_MARGIN_Y = 8;    // margine alto/basso

static inline int textW(const String &s, int size) { return (int)s.length() * FONT_W * size; }
static inline int textH(int size) { return FONT_H * size; }

// setCursor con margine (comodo per elementi a sinistra)
static inline void setCursorM(int x, int y) { displaySetCursor(x + UI_MARGIN_X, y + UI_MARGIN_Y); }

// setCursor allineato a destra (x = bordo destro - margine - larghezza testo)
static inline void setCursorRight(const String& s, int y, int size) {
  int x = SCREEN_WIDTH - UI_MARGIN_X - textW(s, size);
  displaySetCursor(x, y + UI_MARGIN_Y);
  displaySetSize(size);
}

// setCursor in basso a sinistra
static inline void setCursorBL(int size) {
  int y = SCREEN_HEIGHT - UI_MARGIN_Y - textH(size);
  displaySetCursor(UI_MARGIN_X, y);
  displaySetSize(size);
}

// setCursor in basso a destra con una stringa di riferimento
static inline void setCursorBR(const String& s, int size) {
  int y = SCREEN_HEIGHT - UI_MARGIN_Y - textH(size);
  int x = SCREEN_WIDTH - UI_MARGIN_X - textW(s, size);
  displaySetCursor(x, y);
  displaySetSize(size);
}

/* ====== Faccine centrate (senza confliggere con faces.ino) ====== */

// restituisce la faccina in base allo stato (riusa le stringhe globali)
static inline String faceByState_(int s) {
  switch (s % 6) {
    case 0: return netgotchiFace;          // "(-v_v)"
    case 1: return netgotchiFace2;         // "(v_v-)"
    case 2: return netgotchiFaceBlink;     // "( .__.)"
    case 3: return netgotchiFaceHappy;     // "(^=^)"
    case 4: return netgotchiFaceSurprised; // "(o__o)"
    default: return netgotchiFaceSleep;    // "(-__- )"
  }
}

// stampa la faccina centrata orizzontalmente (font size 2) con jitter clampato ai margini
void drawnetgotchiFaceCentered(int state) {
  const int size = 2;
  String face = faceByState_(state);

  int w = textW(face, size);
  int h = textH(size);

  // centro + jitter (moveX) limitato ai margini
  int x = (SCREEN_WIDTH - w) / 2 + moveX;
  if (x < UI_MARGIN_X) x = UI_MARGIN_X;
  if (x > SCREEN_WIDTH - UI_MARGIN_X - w) x = SCREEN_WIDTH - UI_MARGIN_X - w;

  // verticale: un filo sopra il centro
  int y = (SCREEN_HEIGHT - h) / 2 - 6;

  displaySetTextColor(1);
  displaySetSize(size);
  displaySetCursor(x, y);
  displayPrint(face);
}
/* =============================================================== */

// ---------- Schermate ----------
void drawSpace() {
  displayClearDisplay();
  updateAndDrawStars();
  drawUFO();

  if (enableNetworkMode) {
    displayTimeAndDate();
    displayStats();
  } else {
    displayOfflineMode();
  }
  displayDisplay();
  delay(1);
}

void displayRippleSpace() {
  displayClearDisplay();
  drawRipple();
  netgotchi_face();

  if (enableNetworkMode) {
    displayTimeAndDate();
    displayStats();
  } else {
    displayOfflineMode();
  }
  displayDisplay();
  delay(1);
}

void NetworkStats() {
  displayClearDisplay();

  // Titolo stato rete (sinistra)
  displaySetTextColor(1);
  displaySetSize(2);
  setCursorM(0, 0);
  networkStatus = (WiFi.status() == WL_CONNECTED) ? "connected" : "disconnected";
  displayPrint("Net: " + networkStatus);

  // Ping solo una volta per ciclo
  displaySetSize(1);
  setCursorM(0, textH(2) + 2);
  if (scanOnce) {
    IPAddress ip(1, 1, 1, 1);  // Cloudflare
    SerialPrintLn("pinging cloudflare");
    if (Ping.ping(ip, 2)) {
      externalNetworkStatus = "Reachable";
      scanOnce = false;
      stats = "avg: " + String(Ping.averageTime()) + "ms";
      SerialPrintLn("ping sent");
      SerialPrintLn(stats);
    } else {
      externalNetworkStatus = "Unreachable";
      stats = "avg: n/a";
    }
  }
  displayPrint("Speed: " + stats);

  // Internet status (riga sotto)
  setCursorM(0, textH(2) + textH(1) + 6);
  displayPrint("Internet: " + externalNetworkStatus);

  // HUD standard
  displayStats();
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
int maxRadius = 120;
void drawRipple() {
  int radius = (frame * 10) % maxRadius;
  displayDrawCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, radius, 1);
  frame++;
  delay(90);
}

void displayTimeAndDate() {
  timeClient.update();
  String formattedTime = timeClient.getFormattedTime();
  time_t epochTime = timeClient.getEpochTime();
  struct tm* ptm = gmtime((time_t*)&epochTime);
  int currentDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon + 1;
  int currentYear = ptm->tm_year + 1900;

  // Ora in grande
  displaySetTextColor(1);
  displaySetSize(2);
  setCursorM(0, 0);
  displayPrint(formattedTime);

  // Data subito sotto
  displaySetSize(1);
  setCursorM(0, textH(2));
  displayPrintDate("%02d/%02d/%d", currentDay, currentMonth, currentYear);

  // Honeypot in alto a destra
  displaySetSize(1);
  String hp = "Honeypot";
  setCursorRight(hp, 0, 1);
  displayPrint(hp);
  setCursorRight(hp, textH(1), 1);
  displayPrint(honeypotTriggered ? "BREACHED" : "OK");
}

void displayStats() {
  // Basso sinistra: Hosts/VU
  displaySetTextColor(1);
  setCursorBL(1);
  displayPrint("Hosts:" + String(ipnum) + "  VU:" + String(vulnerabilitiesFound));

  // Basso destra: stato scan/idle
  String mode = startScan ? "Scan" : "Idle";
  setCursorBR(mode, 1);
  displayPrint(mode);
}

void displayIPS() {
  displayClearDisplay();
  displaySetTextColor(1);

  // Titolo
  displaySetSize(2);
  setCursorM(0, 0);
  displayPrint("Found Hosts");

  // Lista (font 1, righe da 12px)
  displaySetSize(1);
  int lineY = textH(2) + 6;

  if (ipnum > 0) {
    String ipprefix = String(currentIP[0]) + "." + String(currentIP[1]) + "." + String(currentIP[2]) + ".";
    int shown = 0;

    for (int j = 0; j < max_ip; j++) {
      if (ips[j] == 1 || ips[j] == -1 || ips[j] == 2) {
        if (lineY > SCREEN_HEIGHT - UI_MARGIN_Y - textH(1)) {
          // pagina successiva semplice: ferma qui e lascia HUD
          break;
        }
        setCursorM(0, lineY);
        if      (ips[j] == 1)  displayPrint(ipprefix + String(j) + "  UP");
        else if (ips[j] == 2)  displayPrint(ipprefix + String(j) + "  WARN!");
        else if (ips[j] == -1) displayPrint(ipprefix + String(j) + "  DOWN");
        lineY += 12;
        shown++;
      }
    }
  } else {
    // Nessun host: passa alla prossima schermata
    nextScreen();
  }

  // HUD
  displayStats();
  displayDisplay();
}

void displayNetgotchi() {
  displayClearDisplay();
  updateAndDrawStars();

  if (enableNetworkMode) {
    displayTimeAndDate();
    displayStats();
  } else {
    displayOfflineMode();
  }

  netgotchi_face();
  displayDisplay();
}

void netgotchi_face() {
  // faccina centrata
  drawnetgotchiFaceCentered(animState);

  // animazione tempo + jitter
  if (seconds - old_seconds > 1) {
    moveX = moveX + random(-5, 5);
    if (moveX > 20)  moveX = 5;
    if (moveX < -20) moveX = -5;

    old_seconds = seconds;
    animState = (animState + 1) % 6;
  }

  // ripristina size predefinito per l'HUD
  displaySetSize(1);
}

void displayOfflineMode() {
  setCursorBL(1);
  displayPrint("Netgotchi is Offline");
}

void netgotchiIntro() {
  displayClearDisplay();
  displaySetTextColor(1);

  // Titolo grande
  displaySetSize(2);
  setCursorM(0, 0);
  displayPrint("Netgotchi v." + String(VERSION) + "-C6");

  // Sottotitolo
  displaySetSize(1);
  setCursorM(0, textH(2) + 6);
  displayPrint("Forked by Federicokalik");
  displayDisplay();
  delay(700);
}

void displaySettings() {
  displayClearDisplay();
  displaySetTextColor(1);

  // Titolo
  displaySetSize(2);
  setCursorM(0, 0);
  displayPrint("Settings v." + String(VERSION));

  // Voci (font 1)
  displaySetSize(1);
  int y = textH(2) + 6;
  for (int i = 0; i < settingLength; i++) {
    setCursorM(0, y);
    if (selectedSetting == i) displayPrint(">" + settings[i]);
    else                      displayPrint(" " + settings[i]);
    y += 12;
  }
  displayDisplay();
}

void displayNetgotchiStats() {
  displayClearDisplay();
  displaySetTextColor(1);

  // Titolo
  displaySetSize(2);
  setCursorM(0, 0);
  displayPrint("Netgotchi v." + String(VERSION) + "-C6");

  // Info (font 1, spaziatura 12)
  displaySetSize(1);
  int y = textH(2) + 6;
  setCursorM(0, y);          displayPrint("IP:  " + currentIP.toString()); y += 12;
  setCursorM(0, y);          displayPrint("Uptime: " + String(seconds) + "s"); y += 12;
  if (WiFi.status() == WL_CONNECTED) {
    setCursorM(0, y);        displayPrint("SSID: " + WiFi.SSID()); y += 12;
  }

  // HUD
  displayStats();
  displayDisplay();
}

void screenAnimations() {
  if (animation == 0) displayNetgotchi();
  if (animation == 1) drawSpace();
  if (animation > max_anim) animation = 0;
}

int getPixelAt(int x, int y) {
  if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return -1;
#ifdef USE_ST7789
  // ST7789 non ha framebuffer → WebUI "matrix" ritorna 0 (nero)
  return 0;
#else
  int byteIndex = (y / 8) * SCREEN_WIDTH + x;
  int bitIndex  = y % 8;
  uint8_t* buffer = displayGetBuffer();
  if (!buffer) return 0;
  uint8_t byteValue = buffer[byteIndex];
  bool isWhite = (byteValue & (1 << bitIndex)) != 0;
  return isWhite ? 1 : 0;
#endif
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

void nextScreen() {
  currentScreen++;
  if (currentScreen > maxScreens) {
    currentScreen = 0;
    animation++;
    if (animation > max_anim) animation = 0;
  }
}
