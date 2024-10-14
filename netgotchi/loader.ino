
//loader vars 
int selectedMode = 0;
int availableModeLength=4;
String availableMode[] = {"NETGOTCHI", "TEXTGOTCHI", "CTRLGOTCHI", "DEAUTHERGOTCHI"};
bool loaderSetupSuccess =false;
bool displayLoaderComplete=false;
bool loaderSuccess = false;
long closingSec = 1000; 

Button2 loaderButtonLeft;
Button2 loaderButtonRight;
Button2 loaderButtonA;
Button2 loaderButtonB;


void loaderSetup()
{
  loaderButtonLeft.begin(BTN_LEFT);
  loaderButtonRight.begin(BTN_RIGHT);
  loaderButtonA.begin(BTN_A);
  loaderButtonB.begin(BTN_B);
  loaderButtonLeft.setPressedHandler(leftButtonPressed);
  loaderButtonRight.setPressedHandler(rightButtonPressed);
  loaderButtonA.setPressedHandler(AButtonPressed);
  loaderButtonB.setPressedHandler(BButtonPressed);
  if(skipLoader)SkipLoader();

}

void rightButtonPressed(Button2 &btn) {
    selectedMode++;
    if(selectedMode>=availableModeLength)selectedMode=0;
}

void leftButtonPressed(Button2 &btn) {
    selectedMode--;
    if(selectedMode < 0)selectedMode=availableModeLength-1;
}

void AButtonPressed(Button2 &btn) {
    closingSec=10;
}
void BButtonPressed(Button2 &btn) {
    closingSec=1000;
}

void loader()
{
  if(!displayLoaderComplete )
  {
    bool debounce =true;
    displayClearDisplay();
    displaySetSize(1);
    displaySetTextColor(1);  //white color
    displaySetCursor(0, 0);
    displayPrintln("Gotchi Loader [0_0]");
    displayPrintln(" ");

    for(int i=0; i< availableModeLength ; i++)
    {
      if(selectedMode == i)
      displayPrintln(">"+availableMode[i]);
      else
      displayPrintln(" "+availableMode[i]);
    }
    displayPrintln(" ");
    displayPrint("Closing in ..");
    displayPrintln(String(closingSec));
    displayDisplay();
    delay(20);

    closingSec -= 4;
    if(closingSec <=0 ) 
    {
      loadedSetup();
      displayLoaderComplete=true;
    }
    loaderButtonLeft.loop();
    loaderButtonRight.loop();
    loaderButtonA.loop();
    loaderButtonB.loop();
  }
  else loadedLoop();
}


void loadedSetup()
{

  //all modes
  if(selectedMode == 0){ 
    netgotchi_setup();
  }
  if(selectedMode == 1){ 
    textgotchi_setup();
  }
  if(selectedMode == 2){ 
    ctrlgotchi_setup();
  }
  if(selectedMode == 3){ 
    deauthergotchi_setup();
  }
  //close the setup
  loaderSetupSuccess=true;
}

void loadedLoop()
{
  //run normal loops 
  if(selectedMode == 0 ) netgotchi_loop();
  if(selectedMode == 1 ) textgotchi_loop();
  if(selectedMode == 2 ) ctrlgotchi_loop();
  if(selectedMode == 3 ) deauthergotchi_loop();
}

void SkipLoader()
{
  //closing loader very fast
  closingSec=5;
}