//sounds functions


void playAlert() {
#ifdef ESP32
   //to add a library for Tone
#elif defined(ESP8266)
    tone(buzzer_pin, 500);
    delay(500);
    noTone(buzzer_pin);
    delay(500);
    tone(buzzer_pin, 500);
    delay(500);
    noTone(buzzer_pin);
#endif
}