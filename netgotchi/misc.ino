void countdownToRestart()
{
  //restart in case of connection timeout , retry connection every 5 minutes.
  if(seconds > 360) ESP.restart();
}

void raisePinVoltage()
{
  pinMode(EXT_PIN_16, OUTPUT);
  digitalWrite(EXT_PIN_16, HIGH);
}
void lowerPinVoltage()
{
  pinMode(EXT_PIN_16, OUTPUT);
  digitalWrite(EXT_PIN_16, LOW);
}
