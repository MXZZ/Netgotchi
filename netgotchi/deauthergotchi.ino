//work in progress!

void deauthergotchi_setup()
{
  displayClearDisplay();
  delay(500);
  display.setTextSize(1);
  display.setCursor(0, 5);
  display.println("DEAUTHERGOTCHI");
  display.println("");
  display.println("Work in progress..");
  display.println("Restarting...");
  display.display();
}

void deauthergotchi_loop()
{
    delay(5000);
    ESP.restart();
}