void settingConfirm()
{
  if(selectedSetting == 0)
  {
    wifiManager.autoConnect("AutoConnectAP");
  }
  if(selectedSetting == 1)
  {
    enableNetworkMode = true;
  }
  if(selectedSetting == 2)
  {
    WiFi.disconnect();
    enableNetworkMode = false;
  }
  if(selectedSetting == 3)
  {
    //webinterface code
  }
  if(selectedSetting == 4)
  {
    ESP.restart();
  }
  if(selectedSetting == 5)
  {
    resetSettings();
  }
  settingCancel();
}

void settingCancel()
{
  settingMode = false;
  selectedSetting=0;
}