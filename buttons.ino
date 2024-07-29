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

bool readButton(int pin, unsigned long debounceDelay = 10) {
  static int lastButtonState = HIGH;
  static int buttonState = HIGH;
  static unsigned long lastDebounceTime = 0;

  int reading = digitalRead(pin);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        lastButtonState = reading;
        return true;  // Button is pressed
      }
    }
  }

  lastButtonState = reading;
  return false;  // Button is not pressed
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

  SerialPrintLn(String(btn.getID()));
}