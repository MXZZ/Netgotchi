//Buttons functions

Button2 buttonUp;
Button2 buttonDown;
Button2 buttonLeft;
Button2 buttonRight;
Button2 buttonA;
Button2 buttonB;


void buttonsInit()
{
    //change your pins configurations
    buttonA.begin(2);
    buttonA.setID(2);
    buttonB.begin(0);
    buttonB.setID(0);
    buttonLeft.begin(16);
    buttonLeft.setID(16);
    buttonUp.begin(14);
    buttonUp.setID(14);
    buttonDown.begin(12);
    buttonDown.setID(12);
    buttonRight.begin(13);
    buttonRight.setID(13);

    buttonA.setPressedHandler(buttonPressed);
    buttonB.setPressedHandler(buttonPressed);
    buttonLeft.setPressedHandler(buttonPressed);
    buttonRight.setPressedHandler(buttonPressed);
    buttonUp.setPressedHandler(buttonPressed);
    buttonDown.setPressedHandler(buttonPressed);
}

void buttonLoops() {
  if (digitalRead(flashButtonPin) == LOW) {
    delay(50);
    if (digitalRead(flashButtonPin) == LOW) {
      displayClearDisplay();
      displayPrintln("Flash button pressed. WiFiManager settings...");
      wifiManager.resetSettings();
      displayPrintln("EEPROM and WiFiManager settings erased.");
      displayPrintln("Restart this device");
      delay(10000);
      ESP.restart();
    }
  }
}


void controlsButtonLoop()
{ 
  buttonA.loop();
  buttonB.loop();
  buttonUp.loop();
  buttonDown.loop();
  buttonLeft.loop();
  buttonRight.loop();
}


// Function to handle button press event
void buttonPressed(Button2 &btn) {

  switch (btn.getID()){
    case 2: 
    break;
    case 0:
    break;
    case 16:
    break;
    case 14:
    break;
    case 12:
    break;
    case 13:
    break;
  }
}