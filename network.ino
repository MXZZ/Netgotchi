//Network features 


void pingNetwork(int i) {
  status = "Scanning";
  IPAddress ip = IPAddress(currentIP[0], currentIP[1], currentIP[2], i);
  if (Ping.ping(ip, 1)) {
    SerialPrintLn("Alive");
    SerialPrintLn(ip.toString().c_str());

    iprows++;
    ipnum++;
    ips[i] = 1;

    if (securityScanActive) {
      int scanresult = scanForDangerousServices(ip);
      if (scanresult == 1) {
        SerialPrintLn(String(scanresult));
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


void saveCurrentNetworkInfos()
{
  // Initialize known networks
  numKnownNetworks = 1;
  knownNetworks[0].ssid = WiFi.SSID();
}