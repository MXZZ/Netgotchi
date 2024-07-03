/**
 * This is a kind of unit test for DEV for now
 * It contains many of the public methods
 * 
 */
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <time.h>
#include <stdio.h>

#define USEOTA
// enable OTA
#ifdef USEOTA
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#endif

const char* modes[] = { "NULL", "STA", "AP", "STA+AP" };

unsigned long mtime = 0;


WiFiManager wm;


// TEST OPTION FLAGS
bool TEST_CP         = false; // always start the configportal, even if ap found
int  TESP_CP_TIMEOUT = 90; // test cp timeout

bool TEST_NET        = true; // do a network test after connect, (gets ntp time)
bool ALLOWONDEMAND   = true; // enable on demand
int  ONDDEMANDPIN    = 0; // gpio for button
bool WMISBLOCKING    = true; // use blocking or non blocking mode, non global params wont work in non blocking

uint8_t BUTTONFUNC   = 1; // 0 resetsettings, 1 configportal, 2 autoconnect

// char ssid[] = "*************";  //  your network SSID (name)
// char pass[] = "********";       // your network password


//callbacks
  // called after AP mode and config portal has started
  //  setAPCallback( std::function<void(WiFiManager*)> func );
  // called after webserver has started
  //  setWebServerCallback( std::function<void()> func );
  // called when settings reset have been triggered
  //  setConfigResetCallback( std::function<void()> func );
  // called when wifi settings have been changed and connection was successful ( or setBreakAfterConfig(true) )
  //  setSaveConfigCallback( std::function<void()> func );
  // called when saving either params-in-wifi or params page
  //  setSaveParamsCallback( std::function<void()> func );
  // called when saving params-in-wifi or params before anything else happens (eg wifi)
  //  setPreSaveConfigCallback( std::function<void()> func );
  // called just before doing OTA update
  //  setPreOtaUpdateCallback( std::function<void()> func );

void saveWifiCallback(){
  Serial.println("[CALLBACK] saveCallback fired");
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("[CALLBACK] configModeCallback fired");
  // myWiFiManager->setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); 
  // Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  // Serial.println(myWiFiManager->getConfigPortalSSID());
  // 
  // esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT20);
}

void saveParamCallback(){
  Serial.println("[CALLBACK] saveParamCallback fired");
  // wm.stopConfigPortal();
}

void bindServerCallback(){
  wm.server->on("/custom",handleRoute);

  // you can override wm route endpoints, I have not found a way to remove handlers, but this would let you disable them or add auth etc.
  // wm.server->on("/info",handleNotFound);
  // wm.server->on("/update",handleNotFound);
  wm.server->on("/erase",handleNotFound); // disable erase
}

void handleRoute(){
  Serial.println("[HTTP] handle custom route");
  wm.server->send(200, "text/plain", "hello from user code");
}

void handleNotFound(){
  Serial.println("[HTTP] override handle route");
  wm.handleNotFound();
}

void handlePreOtaUpdateCallback(){
  Update.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("CUSTOM Progress: %u%%\r", (progress / (total / 100)));
  });
}

void setup() {
  // WiFi.mode(WIFI_STA); // explicitly set mode, esp can default to STA+AP
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(3000);
  // Serial.setDebugOutput(true);

  // WiFi.setTxPower(WIFI_POWER_8_5dBm);

  Serial.println("\n Starting");
  // WiFi.setSleepMode(WIFI_NONE_SLEEP); // disable sleep, can improve ap stability

  Serial.println("Error - TEST");
  Serial.println("Information- - TEST");

  Serial.println("[ERROR]  TEST");
  Serial.println("[INFORMATION] TEST");  


  // WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN); // wifi_scan_method_t scanMethod
  // WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL); // wifi_sort_method_t sortMethod - WIFI_CONNECT_AP_BY_SIGNAL,WIFI_CONNECT_AP_BY_SECURITY
  // WiFi.setMinSecurity(WIFI_AUTH_WPA2_PSK);

  wm.setDebugOutput(true, WM_DEBUG_DEV);
  wm.debugPlatformInfo();

  //reset settings - for testing
  // wm.resetSettings();
  // wm.erase();

  // setup some parameters

  WiFiManagerParameter custom_html("<p style=\"color:pink;font-weight:Bold;\">This Is Custom HTML</p>"); // only custom html
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", "", 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", "", 6);
  WiFiManagerParameter custom_token("api_token", "api token", "", 16);
  WiFiManagerParameter custom_tokenb("invalid token", "invalid token", "", 0); // id is invalid, cannot contain spaces
  WiFiManagerParameter custom_ipaddress("input_ip", "input IP", "", 15,"pattern='\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}'"); // custom input attrs (ip mask)
  WiFiManagerParameter custom_input_type("input_pwd", "input pass", "", 15,"type='password'"); // custom input attrs (ip mask)

  const char _customHtml_checkbox[] = "type=\"checkbox\""; 
  WiFiManagerParameter custom_checkbox("my_checkbox", "My Checkbox", "T", 2, _customHtml_checkbox, WFM_LABEL_AFTER);

  const char *bufferStr = R"(
  <!-- INPUT CHOICE -->
  <br/>
  <p>Select Choice</p>
  <input style='display: inline-block;' type='radio' id='choice1' name='program_selection' value='1'>
  <label for='choice1'>Choice1</label><br/>
  <input style='display: inline-block;' type='radio' id='choice2' name='program_selection' value='2'>
  <label for='choice2'>Choice2</label><br/>

  <!-- INPUT SELECT -->
  <br/>
  <label for='input_select'>Label for Input Select</label>
  <select name="input_select" id="input_select" class="button">
  <option value="0">Option 1</option>
  <option value="1" selected>Option 2</option>
  <option value="2">Option 3</option>
  <option value="3">Option 4</option>
  </select>
  )";

  WiFiManagerParameter custom_html_inputs(bufferStr);

  // callbacks
  wm.setAPCallback(configModeCallback);
  wm.setWebServerCallback(bindServerCallback);
  wm.setSaveConfigCallback(saveWifiCallback);
  wm.setSaveParamsCallback(saveParamCallback);
  wm.setPreOtaUpdateCallback(handlePreOtaUpdateCallback);

  // add all your parameters here
  wm.addParameter(&custom_html);
  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_port);
  wm.addParameter(&custom_token);
  wm.addParameter(&custom_tokenb);
  wm.addParameter(&custom_ipaddress);
  wm.addParameter(&custom_checkbox);
  wm.addParameter(&custom_input_type);

  wm.addParameter(&custom_html_inputs);

  // set values later if you want
  custom_html.setValue("test",4);
  custom_token.setValue("test",4);

  // const char* icon = "
  // <link rel='icon' type='image/png' sizes='16x16' href='data:image/png;base64,
  // iVBORw0KGgoAAAANSUhEUgAAABAAAAAQBAMAAADt3eJSAAAAMFBMVEU0OkArMjhobHEoPUPFEBIu
  // O0L+AAC2FBZ2JyuNICOfGx7xAwTjCAlCNTvVDA1aLzQ3COjMAAAAVUlEQVQI12NgwAaCDSA0888G
  // CItjn0szWGBJTVoGSCjWs8TleQCQYV95evdxkFT8Kpe0PLDi5WfKd4LUsN5zS1sKFolt8bwAZrCa
  // GqNYJAgFDEpQAAAzmxafI4vZWwAAAABJRU5ErkJggg==' />";


  // set custom html head content , inside <head>
  // examples of favicon, or meta tags etc
  // const char* headhtml = "<link rel='icon' type='image/png' href='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAADQElEQVRoQ+2YjW0VQQyE7Q6gAkgFkAogFUAqgFQAVACpAKiAUAFQAaECQgWECggVGH1PPrRvn3dv9/YkFOksoUhhfzwz9ngvKrc89JbnLxuA/63gpsCmwCADWwkNEji8fVNgotDM7osI/x777x5l9F6JyB8R4eeVql4P0y8yNsjM7KGIPBORp558T04A+CwiH1UVUItiUQmZ2XMReSEiAFgjAPBeVS96D+sCYGaUx4cFbLfmhSpnqnrZuqEJgJnd8cQplVLciAgX//Cf0ToIeOB9wpmloLQAwpnVmAXgdf6pwjpJIz+XNoeZQQZlODV9vhc1Tuf6owrAk/8qIhFbJH7eI3eEzsvydQEICqBEkZwiALfF70HyHPpqScPV5HFjeFu476SkRA0AzOfy4hYwstj2ZkDgaphE7m6XqnoS7Q0BOPs/sw0kDROzjdXcCMFCNwzIy0EcRcOvBACfh4k0wgOmBX4xjfmk4DKTS31hgNWIKBCI8gdzogTgjYjQWFMw+o9LzJoZ63GUmjWm2wGDc7EvDDOj/1IVMIyD9SUAL0WEhpriRlXv5je5S+U1i2N88zdPuoVkeB+ls4SyxCoP3kVm9jsjpEsBLoOBNC5U9SwpGdakFkviuFP1keblATkTENTYcxkzgxTKOI3jyDxqLkQT87pMA++H3XvJBYtsNbBN6vuXq5S737WqHkW1VgMQNXJ0RshMqbbT33sJ5kpHWymzcJjNTeJIymJZtSQd9NHQHS1vodoFoTMkfbJzpRnLzB2vi6BZAJxWaCr+62BC+jzAxVJb3dmmiLzLwZhZNPE5e880Suo2AZgB8e8idxherqUPnT3brBDTlPxO3Z66rVwIwySXugdNd+5ejhqp/+NmgIwGX3Py3QBmlEi54KlwmjkOytQ+iJrLJj23S4GkOeecg8G091no737qvRRdzE+HLALQoMTBbJgBsCj5RSWUlUVJiZ4SOljb05eLFWgoJ5oY6yTyJp62D39jDANoKKcSocPJD5dQYzlFAFZJflUArgTPZKZwLXAnHmerfJquUkKZEgyzqOb5TuDt1P3nwxobqwPocZA11m4A1mBx5IxNgRH21ti7KbAGiyNn3HoF/gJ0w05A8xclpwAAAABJRU5ErkJggg==' />";
  // const char* headhtml = "<meta name='color-scheme' content='dark light'><style></style><script></script>";
  // wm.setCustomHeadElement(headhtml);

  // set custom html menu content , inside menu item "custom", see setMenu()
  const char* menuhtml = "<form action='/custom' method='get'><button>Custom</button></form><br/>\n";
  wm.setCustomMenuHTML(menuhtml);

  // invert theme, dark
  wm.setDarkMode(true);

  // show scan RSSI as percentage, instead of signal stength graphic
  // wm.setScanDispPerc(true);

/*
  Set cutom menu via menu[] or vector
  const char* menu[] = {"wifi","wifinoscan","info","param","close","sep","erase","restart","exit"};
  wm.setMenu(menu,9); // custom menu array must provide length
*/

  std::vector<const char *> menu = {"wifi","wifinoscan","info","param","custom","close","sep","erase","update","restart","exit"};
  // wm.setMenu(menu); // custom menu, pass vector
  
  // wm.setParamsPage(true); // move params to seperate page, not wifi, do not combine with setmenu!

  // set STA static ip
  // wm.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  // wm.setShowStaticFields(false);
  // wm.setShowDnsFields(false);

  // set AP static ip
  // wm.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
  // wm.setAPStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); 

  // set country
  // setting wifi country seems to improve OSX soft ap connectivity, 
  // may help others as well, default is CN which has different channels

  // wm.setCountry("US"); // crashing on esp32 2.0

  // set Hostname

  // wm.setHostname(("WM_"+wm.getDefaultAPName()).c_str());
  // wm.setHostname("WM_RANDO_1234");

  // set custom channel
  // wm.setWiFiAPChannel(13);
  
  // set AP hidden
  // wm.setAPHidden(true);

  // show password publicly in form
  // wm.setShowPassword(true);

  // sets wether wm configportal is a blocking loop(legacy) or not, use wm.process() in loop if false
  // wm.setConfigPortalBlocking(false);
  
  if(!WMISBLOCKING){
    wm.setConfigPortalBlocking(false);
  }


  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep in seconds
  wm.setConfigPortalTimeout(TESP_CP_TIMEOUT);
  
  // set min quality to show in web list, default 8%
  // wm.setMinimumSignalQuality(50);

  // set connection timeout
  // wm.setConnectTimeout(20);

  // set wifi connect retries
  // wm.setConnectRetries(2);

  // connect after portal save toggle
  // wm.setSaveConnect(false); // do not connect, only save

  // show static ip fields
  // wm.setShowStaticFields(true);
  
  // wm.startConfigPortal("AutoConnectAP", "password");
  
  // This is sometimes necessary, it is still unknown when and why this is needed but it may solve some race condition or bug in esp SDK/lib
  // wm.setCleanConnect(true); // disconnect before connect, clean connect
  
  wm.setBreakAfterConfig(true); // needed to use saveWifiCallback

  // set custom webserver port, automatic captive portal does not work with custom ports!
  // wm.setHttpPort(8080);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration

  // use autoconnect, but prevent configportal from auto starting
  // wm.setEnableConfigPortal(false);

  wifiInfo();

  // to preload autoconnect with credentials
  // wm.preloadWiFi("ssid","password");

  if(!wm.autoConnect("WM_AutoConnectAP","12345678")) {
    Serial.println("failed to connect and hit timeout");
  }
  else if(TEST_CP) {
    // start configportal always
    delay(1000);
    Serial.println("TEST_CP ENABLED");
    wm.setConfigPortalTimeout(TESP_CP_TIMEOUT);
    wm.startConfigPortal("WM_ConnectAP","12345678");
  }
  else {
    //if you get here you have connected to the WiFi
     Serial.println("connected...yeey :)");
  }
  
  wifiInfo();
  pinMode(ONDDEMANDPIN, INPUT_PULLUP);

  #ifdef USEOTA
    ArduinoOTA.begin();
  #endif

}

void wifiInfo(){
  // can contain gargbage on esp32 if wifi is not ready yet
  Serial.println("[WIFI] WIFI_INFO DEBUG");
  WiFi.printDiag(Serial);
  Serial.println("[WIFI] MODE: " + (String)(wm.getModeString(WiFi.getMode())));
  Serial.println("[WIFI] SAVED: " + (String)(wm.getWiFiIsSaved() ? "YES" : "NO"));
  Serial.println("[WIFI] SSID: " + (String)wm.getWiFiSSID());
  Serial.println("[WIFI] PASS: " + (String)wm.getWiFiPass());
  // Serial.println("[WIFI] HOSTNAME: " + (String)WiFi.getHostname());
}

void loop() {

  if(!WMISBLOCKING){
    wm.process();
  }


  #ifdef USEOTA
  ArduinoOTA.handle();
  #endif
  // is configuration portal requested?
  if (ALLOWONDEMAND && digitalRead(ONDDEMANDPIN) == LOW ) {
    delay(100);
    if ( digitalRead(ONDDEMANDPIN) == LOW || BUTTONFUNC == 2){
      Serial.println("BUTTON PRESSED");

      // button reset/reboot
      if(BUTTONFUNC == 0){
        wm.resetSettings();
        wm.reboot();
        delay(200);
        return;
      }
      
      // start configportal
      if(BUTTONFUNC == 1){
        if (!wm.startConfigPortal("OnDemandAP","12345678")) {
          Serial.println("failed to connect and hit timeout");
          delay(3000);
        }
        return;
      }

      //test autoconnect as reconnect etc.
      if(BUTTONFUNC == 2){
        wm.setConfigPortalTimeout(TESP_CP_TIMEOUT);
        wm.autoConnect();
        return;
      }
    
    }
    else {
      //if you get here you have connected to the WiFi
      Serial.println("connected...yeey :)");
      getTime();
    }
  }

  // every 10 seconds
  if(millis()-mtime > 10000 ){
    if(WiFi.status() == WL_CONNECTED){
      getTime();
    }
    else Serial.println("No Wifi");  
    mtime = millis();
  }
  // put your main code here, to run repeatedly:
  delay(100);
}

void getTime() {
  int tz           = -5;
  int dst          = 0;
  time_t now       = time(nullptr);
  unsigned timeout = 5000; // try for timeout
  unsigned start   = millis();
  configTime(tz * 3600, dst * 3600, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync: ");
  while (now < 8 * 3600 * 2 ) { // what is this ?
    delay(100);
    Serial.print(".");
    now = time(nullptr);
    if((millis() - start) > timeout){
      Serial.println("\n[ERROR] Failed to get NTP time.");
      return;
    }
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void debugchipid(){
  // WiFi.mode(WIFI_STA);
  // WiFi.printDiag(Serial);
  // Serial.println(modes[WiFi.getMode()]);
  
  // ESP.eraseConfig();
  // wm.resetSettings();
  // wm.erase(true);
  WiFi.mode(WIFI_AP);
  // WiFi.softAP();
  WiFi.enableAP(true);
  delay(500);
  // esp_wifi_start();
  delay(1000);
  WiFi.printDiag(Serial);
  delay(60000);
  ESP.restart();

  // AP esp_267751
  // 507726A4AE30
  // ESP32 Chip ID = 507726A4AE30
}
