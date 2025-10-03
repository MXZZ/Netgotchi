// ============================================================================
// BMOGOTCHI - BMO Mode for Netgotchi
// ============================================================================
// Displays BMO character with animated facial expressions using shapes
// No button control - fully autonomous and alive!

// Face dimensions and positions (scaled up to fill screen)
#define FACE_X 64          // Center of face
#define FACE_Y 32          // Center of face
#define EYE_WIDTH 18       // Eye rectangle width (bigger!)
#define EYE_HEIGHT 18      // Eye rectangle height (bigger!)
#define PUPIL_SIZE 8       // Pupil square size (bigger!)
#define EYE_SPACING 24     // Distance between eyes
#define MOUTH_WIDTH 40     // Width of mouth
#define MOUTH_HEIGHT 6     // Height of mouth (closed)

// Animation state variables
int bmoEyeLeftX = 0;       // Left eye pupil offset X
int bmoEyeLeftY = 0;       // Left eye pupil offset Y
int bmoEyeRightX = 0;      // Right eye pupil offset X
int bmoEyeRightY = 0;      // Right eye pupil offset Y
int bmoMouthCurve = 0;     // -1=sad, 0=neutral, 1=smile
int bmoMouthOpen = 0;      // 0=closed, 1-5=opening amount
bool bmoIsBlinking = false;
int bmoBlinkProgress = 0;  // 0=open, 10=fully closed

// Timing variables
unsigned long bmoLastBlink = 0;
unsigned long bmoLastEyeMove = 0;
unsigned long bmoLastMouthChange = 0;
unsigned long bmoLastExpression = 0;
unsigned long bmoBlinkInterval = 3000;
unsigned long bmoEyeMoveInterval = 1500;
unsigned long bmoMouthInterval = 2000;
unsigned long bmoExpressionInterval = 5000;

// Current emotional state
int bmoEmotion = 0; // 0=neutral, 1=happy, 2=excited, 3=curious, 4=talking

// Button override control
bool bmoButtonOverride = false;
unsigned long bmoButtonOverrideTime = 0;
const unsigned long BMO_OVERRIDE_DURATION = 2000; // Return to autonomous after 2 seconds

Button2 bmoButtonLeft;
Button2 bmoButtonRight;
Button2 bmoButtonA;
Button2 bmoButtonB;

// ============================================================================
// BUTTON HANDLERS
// ============================================================================

void bmoLeftButtonPressed(Button2 &btn) {
  bmoButtonOverride = true;
  bmoButtonOverrideTime = millis();
  // Look left
  bmoEyeLeftX = -4;
  bmoEyeLeftY = 0;
  bmoEyeRightX = -4;
  bmoEyeRightY = 0;
}

void bmoRightButtonPressed(Button2 &btn) {
  bmoButtonOverride = true;
  bmoButtonOverrideTime = millis();
  // Look right
  bmoEyeLeftX = 4;
  bmoEyeLeftY = 0;
  bmoEyeRightX = 4;
  bmoEyeRightY = 0;
}

void bmoAButtonPressed(Button2 &btn) {
  bmoButtonOverride = true;
  bmoButtonOverrideTime = millis();
  // Smile
  bmoMouthCurve = 1;
  bmoMouthOpen = 0;
}

void bmoBButtonPressed(Button2 &btn) {
  bmoButtonOverride = true;
  bmoButtonOverrideTime = millis();
  // Open mouth
  bmoMouthCurve = 0;
  bmoMouthOpen = 4;
}

// ============================================================================
// SETUP & LOOP
// ============================================================================

void bmogotchi_setup() {
  // Show intro
  displayClearDisplay();
  display.setTextSize(2);
  display.setCursor(20, 15);
  display.println("BMO!");
  display.setTextSize(1);
  display.setCursor(5, 40);
  display.println("Hello friend :)");
  display.display();
  delay(2000);
  
  // Initialize random seed
  randomSeed(analogRead(0));
  
  // Initialize buttons
  bmoButtonLeft.begin(BTN_LEFT);
  bmoButtonRight.begin(BTN_RIGHT);
  bmoButtonA.begin(BTN_A);
  bmoButtonB.begin(BTN_B);
  bmoButtonLeft.setPressedHandler(bmoLeftButtonPressed);
  bmoButtonRight.setPressedHandler(bmoRightButtonPressed);
  bmoButtonA.setPressedHandler(bmoAButtonPressed);
  bmoButtonB.setPressedHandler(bmoBButtonPressed);
}

void bmogotchi_loop() {
  unsigned long currentTime = millis();
  
  // Process button inputs
  bmoButtonLeft.loop();
  bmoButtonRight.loop();
  bmoButtonA.loop();
  bmoButtonB.loop();
  
  // Check if button override should expire
  if (bmoButtonOverride && (currentTime - bmoButtonOverrideTime >= BMO_OVERRIDE_DURATION)) {
    bmoButtonOverride = false;
    // Reset to idle state
    bmoEmotion = 0;
    bmoEyeLeftX = 0;
    bmoEyeLeftY = 0;
    bmoEyeRightX = 0;
    bmoEyeRightY = 0;
    bmoMouthCurve = 0;
    bmoMouthOpen = 0;
  }
  
  // Only run autonomous animations if not overridden by button
  if (!bmoButtonOverride) {
    // Autonomous emotion changes
    if (currentTime - bmoLastExpression >= bmoExpressionInterval) {
      bmoLastExpression = currentTime;
      bmoEmotion = random(0, 5); // Change emotion randomly
    }
    
    // Autonomous blinking
    if (currentTime - bmoLastBlink >= bmoBlinkInterval) {
      bmoLastBlink = currentTime;
      bmoIsBlinking = true;
      bmoBlinkProgress = 0;
      bmoBlinkInterval = random(2000, 5000); // Random blink timing
    }
    
    // Smooth eye movement
    if (currentTime - bmoLastEyeMove >= bmoEyeMoveInterval) {
      bmoLastEyeMove = currentTime;
      
      // Move pupils naturally (limited range to stay in square eyes)
      switch (bmoEmotion) {
        case 0: // Neutral - slow drift
          bmoEyeLeftX = random(-2, 3);
          bmoEyeLeftY = random(-2, 3);
          bmoEyeRightX = bmoEyeLeftX;
          bmoEyeRightY = bmoEyeLeftY;
          break;
        case 1: // Happy - look up slightly
          bmoEyeLeftX = random(-1, 2);
          bmoEyeLeftY = random(-2, 1);
          bmoEyeRightX = bmoEyeLeftX;
          bmoEyeRightY = bmoEyeLeftY;
          break;
        case 2: // Excited - big movements
          bmoEyeLeftX = random(-3, 4);
          bmoEyeLeftY = random(-2, 3);
          bmoEyeRightX = bmoEyeLeftX;
          bmoEyeRightY = bmoEyeLeftY;
          break;
        case 3: // Curious - look around independently
          bmoEyeLeftX = random(-2, 3);
          bmoEyeLeftY = random(-2, 3);
          bmoEyeRightX = random(-2, 3);
          bmoEyeRightY = random(-2, 3);
          break;
        case 4: // Talking - centered
          bmoEyeLeftX = 0;
          bmoEyeLeftY = 0;
          bmoEyeRightX = 0;
          bmoEyeRightY = 0;
          break;
      }
      
      bmoEyeMoveInterval = random(1000, 2500);
    }
    
    // Animate mouth based on emotion
    if (currentTime - bmoLastMouthChange >= bmoMouthInterval) {
      bmoLastMouthChange = currentTime;
      
      switch (bmoEmotion) {
        case 0: // Neutral
          bmoMouthCurve = 0;
          bmoMouthOpen = random(0, 2);
          break;
        case 1: // Happy
          bmoMouthCurve = 1;
          bmoMouthOpen = random(0, 3);
          break;
        case 2: // Excited
          bmoMouthCurve = 1;
          bmoMouthOpen = random(2, 5);
          break;
        case 3: // Curious
          bmoMouthCurve = 0;
          bmoMouthOpen = random(1, 3);
          break;
        case 4: // Talking
          bmoMouthCurve = random(-1, 2);
          bmoMouthOpen = random(2, 6);
          break;
      }
      
      bmoMouthInterval = random(800, 2000);
    }
  }
  
  // Handle blink animation (always runs)
  if (bmoIsBlinking) {
    bmoBlinkProgress += 2;
    if (bmoBlinkProgress >= 20) { // Complete blink cycle
      bmoIsBlinking = false;
      bmoBlinkProgress = 0;
    }
  }
  
  // Draw BMO face
  bmo_drawFace();
  
  delay(50);
}

// ============================================================================
// DRAWING FUNCTIONS
// ============================================================================

void bmo_drawFace() {
  displayClearDisplay();
  
  // Calculate eye positions (centered, larger, cuter)
  int leftEyeX = FACE_X - EYE_SPACING - EYE_WIDTH/2;
  int rightEyeX = FACE_X + EYE_SPACING - EYE_WIDTH/2;
  int eyeY = FACE_Y - 10;
  
  // Draw eyes (square/rectangular eyes like BMO, bigger and cuter)
  if (bmoIsBlinking) {
    // Draw closed eyes as horizontal rectangles
    int closeAmount = (bmoBlinkProgress < 10) ? bmoBlinkProgress : (20 - bmoBlinkProgress);
    int eyeHeight = max(2, EYE_HEIGHT - closeAmount);
    
    if (eyeHeight > 2) {
      display.drawRect(leftEyeX, eyeY + (EYE_HEIGHT - eyeHeight)/2, EYE_WIDTH, eyeHeight, 1);
      display.drawRect(rightEyeX, eyeY + (EYE_HEIGHT - eyeHeight)/2, EYE_WIDTH, eyeHeight, 1);
      // Add double border for cuter look
      display.drawRect(leftEyeX-1, eyeY + (EYE_HEIGHT - eyeHeight)/2 - 1, EYE_WIDTH+2, eyeHeight+2, 1);
      display.drawRect(rightEyeX-1, eyeY + (EYE_HEIGHT - eyeHeight)/2 - 1, EYE_WIDTH+2, eyeHeight+2, 1);
    } else {
      // Fully closed - draw thick lines
      display.fillRect(leftEyeX-1, eyeY + EYE_HEIGHT/2 - 1, EYE_WIDTH+2, 3, 1);
      display.fillRect(rightEyeX-1, eyeY + EYE_HEIGHT/2 - 1, EYE_WIDTH+2, 3, 1);
    }
  } else {
    // Draw open square eyes (BMO style) with double border for cuter look
    display.drawRect(leftEyeX, eyeY, EYE_WIDTH, EYE_HEIGHT, 1);
    display.drawRect(leftEyeX-1, eyeY-1, EYE_WIDTH+2, EYE_HEIGHT+2, 1);
    display.drawRect(rightEyeX, eyeY, EYE_WIDTH, EYE_HEIGHT, 1);
    display.drawRect(rightEyeX-1, eyeY-1, EYE_WIDTH+2, EYE_HEIGHT+2, 1);
    
    // Draw square pupils with offset for looking around
    int pupilLeftX = leftEyeX + (EYE_WIDTH - PUPIL_SIZE)/2 + bmoEyeLeftX;
    int pupilLeftY = eyeY + (EYE_HEIGHT - PUPIL_SIZE)/2 + bmoEyeLeftY;
    int pupilRightX = rightEyeX + (EYE_WIDTH - PUPIL_SIZE)/2 + bmoEyeRightX;
    int pupilRightY = eyeY + (EYE_HEIGHT - PUPIL_SIZE)/2 + bmoEyeRightY;
    
    display.fillRect(pupilLeftX, pupilLeftY, PUPIL_SIZE, PUPIL_SIZE, 1);
    display.fillRect(pupilRightX, pupilRightY, PUPIL_SIZE, PUPIL_SIZE, 1);
  }
  
  // Draw mouth
  bmo_drawMouth();
  
  display.display();
}

void bmo_drawMouth() {
  int mouthY = FACE_Y + 18;
  int mouthX = FACE_X - MOUTH_WIDTH/2;
  
  if (bmoMouthOpen == 0) {
    // Closed mouth - rectangular/geometric shapes (thicker for better visibility)
    if (bmoMouthCurve > 0) {
      // Smile - angled rectangles forming a smile
      display.fillRect(mouthX, mouthY, MOUTH_WIDTH/2 - 1, MOUTH_HEIGHT, 1);
      display.fillRect(mouthX + MOUTH_WIDTH/2 + 1, mouthY - 3, MOUTH_WIDTH/2 - 1, MOUTH_HEIGHT, 1);
      // Add middle segment for smooth curve
      display.fillRect(mouthX + MOUTH_WIDTH/2 - 2, mouthY - 1, 4, MOUTH_HEIGHT, 1);
    } else if (bmoMouthCurve < 0) {
      // Sad - angled rectangles forming frown
      display.fillRect(mouthX, mouthY, MOUTH_WIDTH/2 - 1, MOUTH_HEIGHT, 1);
      display.fillRect(mouthX + MOUTH_WIDTH/2 + 1, mouthY + 3, MOUTH_WIDTH/2 - 1, MOUTH_HEIGHT, 1);
      // Add middle segment for smooth curve
      display.fillRect(mouthX + MOUTH_WIDTH/2 - 2, mouthY + 1, 4, MOUTH_HEIGHT, 1);
    } else {
      // Neutral - straight rectangle (thicker)
      display.fillRect(mouthX, mouthY, MOUTH_WIDTH, MOUTH_HEIGHT, 1);
    }
  } else {
    // Open mouth - rectangular/square opening (BMO style)
    int openHeight = MOUTH_HEIGHT + (bmoMouthOpen * 3);
    
    if (bmoMouthCurve > 0) {
      // Happy open mouth - wider rectangle with smile shape
      display.drawRect(mouthX - 2, mouthY - 3, MOUTH_WIDTH + 4, openHeight + 2, 1);
      display.drawRect(mouthX - 1, mouthY - 2, MOUTH_WIDTH + 2, openHeight, 1);
      // Fill for emphasis
      display.fillRect(mouthX + 1, mouthY, MOUTH_WIDTH - 2, openHeight - 4, 1);
    } else if (bmoMouthCurve < 0) {
      // Sad open mouth
      display.drawRect(mouthX, mouthY + 2, MOUTH_WIDTH, openHeight, 1);
    } else {
      // Normal open mouth - double rectangle for better look
      display.drawRect(mouthX - 1, mouthY - 1, MOUTH_WIDTH + 2, openHeight + 2, 1);
      display.drawRect(mouthX, mouthY, MOUTH_WIDTH, openHeight, 1);
      // Add inner fill for depth
      if (openHeight > 8) {
        display.fillRect(mouthX + 3, mouthY + 3, MOUTH_WIDTH - 6, openHeight - 6, 1);
      }
    }
  }
}

