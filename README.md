# Netgotchi
reversed pwnagotchi : Netgotchi lives to protect your network 

A small arduino .ino script with some great functions to detect intruders or breach in the network, it pings periodically the network and report if any new device. It also has a rudimental "Honeypot" functionality with a rudimental exposed service, once someone reach the honeypot it will trigger an alarm. Please contribute to the repository via pull requests!

# Requirements 
- ESP8266 ( any )
- SSD1306 display ( ic version ) 
- Usb cable 
- optional 3d printed parts ( from thingiverse.com)

  or get an assembled one here on the [official shop *Ollestore* on Etsy](https://ollestore.etsy.com) 

# How to use
1. open .ino file , choose your subnet ( just change the value or add your own)

Current supported subnets :
//Type of Subnet supported
//192.168.0.0/24 = type 0
//192.168.1.0/24 = type 1
//192.168.88.0/24 = type 2
//192.168.100.0/24  = type 3

2. Install libraries and flash the file in the esp8266 ( connect the display first)
3. On first status WiFiManager library will create an hostpot where you can set your wifi credentials
   WiFiManager wifiManager;
  if (wifiManager.autoConnect("AutoConnectAP")) {
    display.println("Connection Successful");
  } else {
    display.println("Select Wifi AutoConnectAP");
    display.println("to run Wifi Setup");
  }

4. Netgotchi now lives in your network, it will scan periodically using pings and report potential breach of the honeypot
   Enjoy your Netgotchi !
   Please contribuite to the project , let's save our network! 

