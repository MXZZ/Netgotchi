#include <espnow.h>

String command[] = {"< ESPNOW RECEIVER >","< ESPNOW SENDER >", "< ON >", "< OFF >" , "< TIMER 1min >", "< TIMER 15min >", "< TIMER 1h >", "< TIMER 8h >"," < ALERT REPEAT 1h>"," < ALERT REPEAT 30m>","< ALERT REPEAT 1m>" };
String ctrlmessage = "";
int commandLength=11;
int ctrlselectedMode = 0;

String ctrlface = "(éwè)";
unsigned long ctrlpreviousMillis = 0;
const long ctrlinterval = 3000;  // timer at which to change the face
String ctrlstatus = "__";
bool remote_controlled_status=false;
unsigned long previousMillisctrl = 0;
unsigned long currentMillisAlert=0;
unsigned long previousMillisctrlAlert = 0;
bool ctrltimerstarted=false;

struct_message ctrlmyData;

Button2 ctrlloaderButtonLeft;
Button2 ctrlloaderButtonRight;
Button2 ctrlloaderButtonA;
Button2 ctrlloaderButtonB;

long ctrltimer = 0;
long ctrltimerAlert = 3600000; //1h timer for repeating alert

void ctrlLeftButtonPressed(Button2 &btn) {
   ctrlselectedMode--;
    if(ctrlselectedMode< 0 )ctrlselectedMode=commandLength-1;
}

void ctrlRightButtonPressed(Button2 &btn) {
  ctrlselectedMode++;
    if(ctrlselectedMode>=commandLength)ctrlselectedMode=0;
}

void ctrlAButtonPressed(Button2 &btn) {
  playTone();
   if(ctrlselectedMode == 0)
    {
      //display.println("Press A to control");
    }
    if(ctrlselectedMode == 1)
    {
      //display.println("Press A to send ON");
      crtlgotchi_sendMessage("CTRL:TRIGGER");
    }
    if(ctrlselectedMode == 2)
    {
      raisePinVoltage();
      ctrlstatus="TRG";
    }
    if(ctrlselectedMode == 3)
    {
      lowerPinVoltage();
      ctrlstatus="LWR";

    }
    if(ctrlselectedMode == 4)
    {
      ctrltimer=60 * 1000;
    }
    if(ctrlselectedMode == 5)
    {
      ctrltimer=5 *60 * 1000;
    }
    if(ctrlselectedMode == 6)
    {
      ctrltimer=60 *60 * 1000;
    }
    if(ctrlselectedMode == 7)
    {
      ctrltimer=8*60 *60 * 1000;
    }
    if(ctrlselectedMode == 8)
    {
      ctrltimerAlert=3600000;
    }
    if(ctrlselectedMode == 9)
    {
      ctrltimerAlert=1800000;
    }
    if(ctrlselectedMode == 10)
    {
      ctrltimerAlert=60000;
    }
}
void ctrlBButtonPressed(Button2 &btn) {
  playTone();

}


void ctrlgotchi_setup()
{
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  esp_now_register_recv_cb(crtlgotchi_OnDataRecv);

  displayClearDisplay();

  ctrlloaderButtonLeft.begin(BTN_LEFT);
  ctrlloaderButtonRight.begin(BTN_RIGHT);
  ctrlloaderButtonA.begin(BTN_A);
  ctrlloaderButtonB.begin(BTN_B);
  ctrlloaderButtonLeft.setPressedHandler(ctrlLeftButtonPressed);
  ctrlloaderButtonRight.setPressedHandler(ctrlRightButtonPressed);
  ctrlloaderButtonA.setPressedHandler(ctrlAButtonPressed);
  ctrlloaderButtonB.setPressedHandler(ctrlBButtonPressed);
}

void ctrlgotchi_loop()
{
  crtlgotchi_updateDisplay();
  crtlgotchi_loopFace();
  
  ctrlloaderButtonLeft.loop();
  ctrlloaderButtonRight.loop();
  ctrlloaderButtonA.loop();
  ctrlloaderButtonB.loop();

  if(ctrltimer > 0 ){
    unsigned long currentMillisctrl = millis();
    if(!ctrltimerstarted) {
      ctrltimerstarted = true;
      previousMillisctrl=millis();
    }
    if (currentMillisctrl - previousMillisctrl >= ctrltimer) {
      // Enable pin 0 after the interval
      raisePinVoltage();
      ctrltimer=0;
      ctrlstatus="TRG";
      ctrltimerstarted=false;

    }
  }

  if(remote_controlled_status && ctrlselectedMode == 0)
  {
    raisePinVoltage();
    ctrlstatus="TRG";

  }
   if(!remote_controlled_status && ctrlselectedMode == 0)
  {
    lowerPinVoltage();
    ctrlstatus="LWR";

  }

  if(ctrlselectedMode == 8)
    {
        //repeating message
        currentMillisAlert = millis();
        if (currentMillisAlert - previousMillisctrlAlert >= ctrltimerAlert) {
        crtlgotchi_sendMessage("CTRL:ALERT");
        previousMillisctrlAlert= currentMillisAlert;
      }
    }
}

void crtlgotchi_loopFace() {
  unsigned long currentMillis = millis();
  if (currentMillis - ctrlpreviousMillis >= ctrlinterval) {
    ctrlpreviousMillis = currentMillis;
    crtlgotchi_changeFace();
  }
}

void crtlgotchi_changeFace() {
  static int index = 0;
  // Array of different emoticons
  const String ctrlfaces[] = {
    " (v^v) ",
    " (^-^) ",
    "(-_-) ",
    "(.__.) ",
    " (p_p) ",
    " (._.)",
    "(,_,) ",
    " (o_o) "
  };
  // Update the face variable
  ctrlface = ctrlfaces[index];
  // Move to the next ctrlface in the array
  index = (index + 1) % (sizeof(ctrlfaces) / sizeof(ctrlfaces[0]));  // Loop back to the start when reaching the end
}




void crtlgotchi_sendMessage(const String &msg) {

  msg.toCharArray(ctrlmyData.text, sizeof(ctrlmyData.text));

  ctrlstatus = esp_now_send(broadcastAddress, (uint8_t *)&ctrlmyData, sizeof(ctrlmyData)) == 0 ?  "OK" : "ER";
  Serial.println("status:");
  Serial.println(ctrlstatus);
}

void crtlgotchi_OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  display.clearDisplay();  // Clear the display
  ctrlmessage = "";  // Reset the received message
  memcpy(&ctrlmyData, incomingData, sizeof(ctrlmyData));
  ctrlmessage = String(ctrlmyData.text);
  if(ctrlmessage=="CTRL:TRIGGER" && ctrlselectedMode == 0)
  {
    playTone();  // Play tone when a new message is received
    remote_controlled_status = !remote_controlled_status;
  }
}

void crtlgotchi_updateDisplay() {
  display.clearDisplay();
  // Display received and sent messages at the top
  display.setTextSize(1);
  display.setCursor(0, 5);
  display.print("CTRLGOTCHI " + ctrlface + "" + ctrlstatus);
  display.setCursor(0, 15);
  display.print("PIN D0 CONTROL :");
  display.println(ctrlmessage);
  display.println(" ");
  displayPrintln(command[ctrlselectedMode]);
  display.println(" ");

   if(ctrlselectedMode == 0)
    {
      display.println("D0 Control enabled ");
    }
    if(ctrlselectedMode == 1)
    {
      display.println("Press A to send ON/OFF");
    }
    if(ctrlselectedMode == 2)
    {
      display.println("Press A to enable D0");
    }
    if(ctrlselectedMode == 3)
    {
      display.println("Press A to disable D0");
    }
    if(ctrlselectedMode == 4)
    {
      display.println("D0 timer - 1min");
    }
    if(ctrlselectedMode == 5)
    {
      display.println("D0 timer - 15min");
    }
    if(ctrlselectedMode == 6)
    {
      display.println("D0 timer - 1h");
    }
    if(ctrlselectedMode == 7)
    {
      display.println("D0 timer - 8h");
    }
    if(ctrlselectedMode == 8)
    {
      display.println("ALERT each hour");
    }
  
  display.display();
}



