//sounds functions


void playAlert() {
#ifdef ESP32
   //to add a library for Tone
#elif defined(ESP8266)
    tone(BUZZER_PIN, 500);
    delay(500);
    noTone(BUZZER_PIN);
    delay(500);
    tone(BUZZER_PIN, 500);
    delay(500);
    noTone(BUZZER_PIN);
#endif
}

void playTone() {
   tone(BUZZER_PIN,1000 ,100 ); // Play a tone at frequency of1000 Hz for200 ms 
}