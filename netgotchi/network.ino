//Network features 

void networkInit()
{
  
  if (useWifiManager) {
    displayPrintln("TO Configure WIFI");
    displayPrintln("USE: AutoConnectAP");
    displayDisplay();
    } else {
    displayPrintln("Connecting to WiFi");
    displayDisplay();
  }

  if (useWifiManager) {
    wifiManager.setConfigPortalTimeout(wifiManagertimeout);
    if(!wifiManager.autoConnect("AutoConnectAP")) {
      // fail to connect -- timeout
      displayPrintln("Connection Timeout");
      displayPrintln("Restarting in 5min");
      delay(5000);
      scheduledRestart = true;
      enableNetworkMode = false;
    }
    else
    {
      //connection successfull 
      displayPrintln("Connection Successful");
      displayDisplay();
    }

  } else {
    //use normal wifi credential 
    WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      displayPrint(".");
      displayDisplay();
    }
  }

  if(webInterface)
  { 
    server.on("/", handleRoot);

    server.on("/matrix", HTTP_GET, [](){
      if(hasDisplay)server.send(200, "application/json", getPixelMatrix());
      else server.send(404);
    });

    server.on("/hosts", HTTP_GET, [](){
        server.send(200, "text/html", getHostsStats());
    });

    server.on("/headless", HTTP_GET, [](){
        server.send(200, "text/html", headlessStatus);
    });

    server.on("/command/left", HTTP_GET, [](){
        handleCommand("left");
        server.send(200, "text/plain", "Left command received");
    });

    server.on("/command/right", HTTP_GET, [](){
        handleCommand("right");
        server.send(200, "text/plain", "Right command received");
    });

    server.on("/command/A", HTTP_GET, [](){
        handleCommand("A");
        server.send(200, "text/plain", "A command received");
    });

    server.on("/command/B", HTTP_GET, [](){
        handleCommand("B");
        server.send(200, "text/plain", "B command received");
    });
    server.on("/command/ON", HTTP_GET, [](){
        raisePinVoltage();
        server.send(200, "text/plain", "ON command received");
    });
    server.on("/command/OFF", HTTP_GET, [](){
        lowerPinVoltage();
        server.send(200, "text/plain", "OFF command received");
    });
    server.on("/command/TIMEPLUS", HTTP_GET, [](){
        timeOffset+=3600;
        timeClient.setTimeOffset(timeOffset);
        server.send(200, "text/plain", "Hour+ command received");
    });
    server.on("/command/TIMEMINUS", HTTP_GET, [](){
        timeOffset-=3600;
        timeClient.setTimeOffset(timeOffset);
        server.send(200, "text/plain", "Hour- command received");
    });

    server.begin();
  }


  
  currentIP = WiFi.localIP();
  SerialPrintLn(currentIP);
  timeClient.begin();

  ftpSrv.begin("admin", "admin");  // Set FTP username and password
  
}

void networkFunctionsLoop()
{

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

  // Handle webInterface requests
  if(webInterface) server.handleClient();

}

void pingNetwork(int i) {
  status = "Scanning";
  IPAddress ip = IPAddress(currentIP[0], currentIP[1], currentIP[2], i);
  if (Ping.ping(ip, 1)) {
    SerialPrintLn("Alive");
    SerialPrintLn(ip);

    iprows++;
    ipnum++;
    ips[i] = 1;

    if (securityScanActive) {
      int scanresult = scanForDangerousServices(ip);
      if (scanresult == 1) {
        ips[i] = 2;
      }
    }
  } else {
    if (ips[i] == -1) ips[i] = 0;
    else if (ips[i] == 1) ips[i] = -1;
    else if (ips[i] == 2) ips[i] = -1;
    else ips[i] = 0;
  }
}


bool detectEvilTwin() {
  int numNetworks = WiFi.scanNetworks();
  int ssid_count = 0;
  bool currentEvilTwinStatus = false;

  for (int i = 0; i < numNetworks; i++) {
    String ssid = WiFi.SSID(i);

    if (ssid == knownNetworks[0].ssid) {
        ssid_count++;
    }
  }
  if (ssid_count> 1 ) currentEvilTwinStatus = true;

  if (currentEvilTwinStatus != evilTwinDetected) {
    if (currentEvilTwinStatus) {
      SerialPrintLn("Evil Twin appeared");
    } else {
      SerialPrintLn("Evil Twin disappeared");
    }
  }
  return currentEvilTwinStatus;
}


int scanForDangerousServices(IPAddress ip) {
  WiFiClient client;
  for (int i = 0; i < sizeof(dangerousServices) / sizeof(dangerousServices[0]); ++i) {
    if (skipFTPScan && dangerousServices[i].name == "FTP") continue;
    if (client.connect(ip, dangerousServices[i].port)) {
      SerialPrintLn("Open port found: ");
      SerialPrintLn(dangerousServices[i].name);
      SerialPrintLn(" (");
      SerialPrintLn(dangerousServices[i].port);
      SerialPrintLn(") on ");
      SerialPrintLn(ip);
      client.stop();
      vulnerabilitiesFound++;
      return 1;
    }
  }
  return 0;
}


void ftpHoneypotScan() {
  ftpSrv.handleFTP();
  #if defined(ESP8266)
    if (ftpSrv.returnHoneypotStatus()) {
      honeypotTriggered = true;
      delay(500);
    }
  #else
    if (ftpSrv.isClientConnected()) {
      honeypotTriggered = true;
      delay(500);
    }
  #endif
}

void handleRoot() {
  server.send(200, "text/html", pagehtml);
}

void handleCommand(String command) {
    if (command == "left") {
       handleButtons(BTN_LEFT);
    } else if (command == "right") {
       handleButtons(BTN_RIGHT);
    } else if (command == "A") {
       handleButtons(BTN_A);
    } else if (command == "B") {
       handleButtons(BTN_A);
    }
}

void saveCurrentNetworkInfos()
{
  // Initialize known networks
  numKnownNetworks = 1;
  knownNetworks[0].ssid = WiFi.SSID();
}


String getHostsStats() {

  String list = "<br>";
  String ipprefix = String(currentIP[0]) + "." + String(currentIP[1]) + "." + String(currentIP[2]) + ".";
  for (int j = 0; j < max_ip; j++) {
    if (ips[j] != 0) {

      if (ips[j] == 1) {
        list += ipprefix + String(j) + " UP" + "<br>";
      }
      if (ips[j] != 1 && ips[j] != -1) {
        list += ipprefix + String(j) + " WRNG!" + "<br>";
      }
      if (ips[j] == -1) {
        list +=  ipprefix + String(j) + " DOWN" + "<br>";
      }
    }
  }
  return list;
}




