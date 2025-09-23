#include "globals.h"

const char keyboard_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";
int selected_keyboard_index = 0;
String textmessage = "";
String textreceivedMessage = "";
String face = "(=*w*=)";
unsigned long textgotchipreviousMillis = 0;
const long textgotchinterval = 3000;  // textgotchinterval at which to change the face
String textgotchistatus = "__";
unsigned long previousMessageRecTime = 0;


struct_message myData;

Button2 textgotchiloaderButtonLeft;
Button2 textgotchiloaderButtonRight;
Button2 textgotchiloaderButtonA;
Button2 textgotchiloaderButtonB;

void textgotchiLeftButtonPressed(Button2 &btn) {
  selected_keyboard_index = (selected_keyboard_index - 1 + sizeof(keyboard_chars) - 1) % (sizeof(keyboard_chars) - 1);
}

void textgotchiRightButtonPressed(Button2 &btn) {
  selected_keyboard_index = (selected_keyboard_index + 1) % (sizeof(keyboard_chars) - 1);
}

void textgotchiAButtonPressed(Button2 &btn) {
    textmessage += keyboard_chars[selected_keyboard_index];

}
void textgotchiBButtonPressed(Button2 &btn) {
    sendMessage(textmessage);
    textmessage = "";  // Clear message after sending
    playTone();  // Play tone when message is sent
}


void textgotchi_setup()
{
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
esp_now_peer_info_t peer = {};
memcpy(peer.peer_addr, broadcastAddress, 6);
peer.ifidx   = WIFI_IF_STA;   // o WIFI_IF_AP se stai in AP
peer.channel = 1;             // metti il tuo canale Wi-Fi se vuoi allinearlo
peer.encrypt = false;
esp_now_add_peer(&peer);

  esp_now_register_recv_cb(OnDataRecv);

  displayClearDisplay();

  textgotchiloaderButtonLeft.begin(BTN_LEFT);
  textgotchiloaderButtonRight.begin(BTN_RIGHT);
  textgotchiloaderButtonA.begin(BTN_A);
  textgotchiloaderButtonB.begin(BTN_B);
  textgotchiloaderButtonLeft.setPressedHandler(textgotchiLeftButtonPressed);
  textgotchiloaderButtonRight.setPressedHandler(textgotchiRightButtonPressed);
  textgotchiloaderButtonA.setPressedHandler(textgotchiAButtonPressed);
  textgotchiloaderButtonB.setPressedHandler(textgotchiBButtonPressed);
}

void textgotchi_loop()
{
  updateDisplay();
  loopFace();
  handleSerialInput();

  textgotchiloaderButtonLeft.loop();
  textgotchiloaderButtonRight.loop();
  textgotchiloaderButtonA.loop();
  textgotchiloaderButtonB.loop();
}

void loopFace() {
  unsigned long currentMillis = millis();
  if (currentMillis - textgotchipreviousMillis >= textgotchinterval) {
    textgotchipreviousMillis = currentMillis;
    changeFace();
  }
}

void changeFace() {
  static int index = 0;
  // Array of different emoticons
  const String faces[] = {
    "(=*w*=)",
    " (^_^) ",
    "(o_O) ",
    "(>__<) ",
    " (^o^) ",
    " (-_-)",
    "(T_T) ",
    " (^_~) "
  };
  // Update the face variable
  face = faces[index];
  // Move to the next face in the array
  index = (index + 1) % (sizeof(faces) / sizeof(faces[0]));  // Loop back to the start when reaching the end
}




void sendMessage(const String &msg) {

  msg.toCharArray(myData.text, sizeof(myData.text));

  textgotchistatus = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData)) == 0 ?  "OK" : "ER";
  Serial.println("message sent: " + textreceivedMessage);
  Serial.println("status:");
  Serial.println(textgotchistatus);
}

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  display.clearDisplay();  // Clear the display
  textreceivedMessage = "";  // Reset the received message
  memcpy(&myData, incomingData, sizeof(myData));
  textreceivedMessage = String(myData.text);
  Serial.println("Serial message received: " + textreceivedMessage);
  previousMessageRecTime = millis(); // time of last recv message
  playTone();  // Play tone when a new message is received
}

void updateDisplay() {
  display.clearDisplay();
  // Display received and sent messages at the top
  display.setTextSize(1);
  display.setCursor(5, 5);
  display.print("TEXTGOTCHI " + face + "" + textgotchistatus);
  display.setCursor(0, 15);
  display.print("Recv: ");
  display.println(textreceivedMessage);
  display.print("Sent: ");
  display.println(textmessage);
  int secago=(millis() - previousMessageRecTime )/1000;
  display.setCursor(60, 55);
  display.print(" ("+String(secago) + "s ago)");

  // Display keyboard at the bottom

  for (int i = 0; i < sizeof(keyboard_chars) - 1; i++) {
    if (i == selected_keyboard_index) {
      display.setTextColor(0, 1);  // Highlight selected character
    } else {
      display.setTextColor(1);
    }
    int yOffset = (i < 26) ? SCREEN_HEIGHT - 20 : SCREEN_HEIGHT - 10;  // Adjust y position for second line
    int xOffset = (i % 26) * 5;  // Adjust x position based on character index
    display.setCursor(xOffset, yOffset);
    display.print(keyboard_chars[i]);
  }
  display.display();
}

void handleSerialInput() {
  if (Serial.available()) {
    String serialMessage = Serial.readStringUntil('\n');
    serialMessage.trim();  // Remove any trailing newline or spaces
    if (serialMessage.length() > 0) {
      sendMessage(serialMessage);
      Serial.println("Serial message sent: " + serialMessage);
      playTone();  // Play tone when message is sent via serial
    }
  }
}
