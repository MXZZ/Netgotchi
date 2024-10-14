//Buttons functions

Button2 buttonLeft;
Button2 buttonRight;
Button2 buttonA;
Button2 buttonB;


void buttonsInit() {
  //change your pins configurations
  buttonA.begin(BTN_A);
  buttonA.setID(BTN_A);
  buttonB.begin(BTN_B);
  buttonB.setID(BTN_B);
  buttonLeft.begin(BTN_LEFT);
  buttonLeft.setID(BTN_LEFT);
  buttonRight.begin(BTN_RIGHT);
  buttonRight.setID(BTN_RIGHT);

  buttonA.setPressedHandler(buttonPressed);
  buttonB.setPressedHandler(buttonPressed);
  buttonLeft.setPressedHandler(buttonPressed);
  buttonRight.setPressedHandler(buttonPressed);
}

void buttonLoops() {
  if (digitalRead(flashButtonPin) == LOW) {
    delay(50);
    if (digitalRead(flashButtonPin) == LOW) {
      resetSettings();
    }
  }
}

void checkOfflineMode() {

  if (digitalRead(BTN_A) == LOW) {
    delay(50);
    if (digitalRead(BTN_A) == LOW) {
      enableNetworkMode = false;
    }
  }
}

void resetSettings() {
  displayClearDisplay();
  displayPrintln("Flash button pressed. WiFiManager settings...");
  wifiManager.resetSettings();
  displayPrintln("EEPROM and WiFiManager settings erased.");
  displayPrintln("Restart this device");
  delay(10000);
  ESP.restart();
}

void controlsButtonLoop() {
  buttonA.loop();
  buttonB.loop();
  buttonLeft.loop();
  buttonRight.loop();
}


// Function to handle button press event
void buttonPressed(Button2 &btn) {
  //SerialPrintLn(String(btn.getID()));
  handleButtons(btn.getID());
}

void handleButtons(int btnID) {
  switch (btnID) {
    case BTN_A:
      //A
      if (settingMode) settingConfirm();
      break;

    case BTN_B:
      //B
      if (settingMode) settingCancel();
      break;

    case BTN_LEFT:
      //RIGHT
      nextScreen();
      break;

    case BTN_RIGHT:
      //DOWN
      settingMode = true;
      if (settingMode) {
        selectedSetting++;
        if (selectedSetting > settingLength) selectedSetting = 0;
      }
      break;
  }
}
