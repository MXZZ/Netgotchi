// ============================================================================
// GOTCHI LOADER - Multi-mode bootloader with scrolling support
// ============================================================================

// Loader configuration
int selectedMode = 0;
int scrollOffset = 0;  // For scrolling through entries
const int MAX_VISIBLE_ENTRIES = 4;  // Maximum entries visible on screen

// Available modes (add more here!)
String availableMode[] = {
  "NETGOTCHI", 
  "TEXTGOTCHI", 
  "CTRLGOTCHI", 
  "DEAUTHERGOTCHI",
  "BMOGOTCHI"
};
int availableModeLength = sizeof(availableMode) / sizeof(availableMode[0]);

// Loader state
bool loaderSetupSuccess = false;
bool displayLoaderComplete = false;
bool loaderSuccess = false;
long closingSec = 1000; 

Button2 loaderButtonLeft;
Button2 loaderButtonRight;
Button2 loaderButtonA;
Button2 loaderButtonB;


// ============================================================================
// LOADER INITIALIZATION
// ============================================================================

void loaderSetup() {
  loaderButtonLeft.begin(BTN_LEFT);
  loaderButtonRight.begin(BTN_RIGHT);
  loaderButtonA.begin(BTN_A);
  loaderButtonB.begin(BTN_B);
  loaderButtonLeft.setPressedHandler(leftButtonPressed);
  loaderButtonRight.setPressedHandler(rightButtonPressed);
  loaderButtonA.setPressedHandler(AButtonPressed);
  loaderButtonB.setPressedHandler(BButtonPressed);
  
  if (skipLoader) {
    SkipLoader();
  }
}

// ============================================================================
// BUTTON HANDLERS
// ============================================================================

void rightButtonPressed(Button2 &btn) {
  if (hasControlsButtons) {
    selectedMode++;
    
    // Wrap around to beginning
    if (selectedMode >= availableModeLength) {
      selectedMode = 0;
      scrollOffset = 0;
    }
    
    // Auto-scroll down if selection goes below visible area
    if (selectedMode >= scrollOffset + MAX_VISIBLE_ENTRIES) {
      scrollOffset++;
    }
  }
}

void leftButtonPressed(Button2 &btn) {
  if (hasControlsButtons) {
    selectedMode--;
    
    // Wrap around to end
    if (selectedMode < 0) {
      selectedMode = availableModeLength - 1;
      scrollOffset = max(0, availableModeLength - MAX_VISIBLE_ENTRIES);
    }
    
    // Auto-scroll up if selection goes above visible area
    if (selectedMode < scrollOffset) {
      scrollOffset--;
    }
  }
}

void AButtonPressed(Button2 &btn) {
  // Confirm selection - close loader quickly
  closingSec = 10;
}

void BButtonPressed(Button2 &btn) {
  // Cancel - reset timer
  closingSec = 1000;
}

// ============================================================================
// MAIN LOADER DISPLAY
// ============================================================================

void loader() {
  if (!displayLoaderComplete) {
    displayClearDisplay();
    displaySetSize(1);
    displaySetTextColor(1);
    displaySetCursor(0, 0);
    
    // Header
    displayPrintln("Gotchi Loader [0_0]");
    
    // Show scroll up indicator if there are items above
    if (scrollOffset > 0) {
      displayPrintln("      /\\  MORE");
    } else {
      displayPrintln(" ");
    }
    
    // Calculate visible range
    int visibleStart = scrollOffset;
    int visibleEnd = min(scrollOffset + MAX_VISIBLE_ENTRIES, availableModeLength);
    
    // Display visible entries
    for (int i = visibleStart; i < visibleEnd; i++) {
      String prefix = (selectedMode == i) ? ">" : " ";
      displayPrintln(prefix + availableMode[i]);
    }
    
    // Fill remaining lines if less than MAX_VISIBLE_ENTRIES
    for (int i = visibleEnd - visibleStart; i < MAX_VISIBLE_ENTRIES; i++) {
      displayPrintln(" ");
    }
    
    // Show scroll down indicator if there are items below
    if (visibleEnd < availableModeLength) {
      displayPrintln("      \\/  MORE");
    } else {
      displayPrintln(" ");
    }
    
    // Footer with timer
    displayPrint("Closing in ..");
    displayPrintln(String(closingSec));
    displayDisplay();
    delay(20);
    
    // Countdown timer
    closingSec -= 4;
    if (closingSec <= 0) {
      loadedSetup();
      displayLoaderComplete = true;
    }
    
    // Process button inputs
    loaderButtonLeft.loop();
    loaderButtonRight.loop();
    loaderButtonA.loop();
    loaderButtonB.loop();
  } else {
    loadedLoop();
  }
}

// ============================================================================
// MODE SETUP & EXECUTION
// ============================================================================

void loadedSetup() {
  // Initialize the selected mode
  switch (selectedMode) {
    case 0:
      netgotchi_setup();
      break;
    case 1:
      textgotchi_setup();
      break;
    case 2:
      ctrlgotchi_setup();
      break;
    case 3:
      deauthergotchi_setup();
      break;
    case 4:
      bmogotchi_setup();
      break;
    default:
      // Fallback to netgotchi if invalid mode
      netgotchi_setup();
      break;
  }
  
  loaderSetupSuccess = true;
}

void loadedLoop() {
  // Run the selected mode's main loop
  switch (selectedMode) {
    case 0:
      netgotchi_loop();
      break;
    case 1:
      textgotchi_loop();
      break;
    case 2:
      ctrlgotchi_loop();
      break;
    case 3:
      deauthergotchi_loop();
      break;
    case 4:
      bmogotchi_loop();
      break;
    default:
      // Fallback to netgotchi if invalid mode
      netgotchi_loop();
      break;
  }
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

void SkipLoader() {
  // Skip loader by setting timer to almost zero
  closingSec = 5;
}