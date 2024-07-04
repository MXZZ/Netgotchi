# Netgotchi
reversed pwnagotchi : Netgotchi lives to protect your network 

A small arduino .ino script with some great functions to detect intruders or breach in the network, it pings periodically the network and report if any new device. It also has a rudimental "Honeypot" functionality with a rudimental exposed service, once someone reach the honeypot it will trigger an alarm. Please contribute to the repository via pull requests!

*Tested with nmap, service scan / intense scan will trigger the alert.* 

Get an official build from [official shop *Ollestore* on Etsy](https://ollestore.etsy.com/listing/1742406142) and support the development! 

![il_1588xN 6101306530_cn6d](https://github.com/MXZZ/Netgotchi/assets/3322271/5b098ac4-e024-42a9-ae83-8f6892d19dfb)


# Requirements 
- ESP8266 ( any )
- SSD1306 display ( ic version ) 
- Usb cable 
- optional 3d printed parts ( from thingiverse.com)

  or get an assembled one here on the [official shop *Ollestore* on Etsy](https://ollestore.etsy.com/listing/1742406142) 

# Libraries
- It uses a modified version of ESP8266FtpServer ( replace your local one in Arduino>Libraries with this one)
- Adafruit_SSD1306 standard ( install from Library manager)
- ESPping Standard ( install from Library manager)
- WiFiManager standard ( install from Library manager)

# How to use
1. open .ino file , choose your subnet ( just change the value or add your own)

Current supported subnets ( or add your own) :
- 192.168.0.0/24 = type 0
- 192.168.1.0/24 = type 1
- 192.168.88.0/24 = type 2
- 192.168.100.0/24  = type 3

2. Install libraries and flash the file in the esp8266 ( connect the display first)
3. On first status WiFiManager library will create an hostpot where you can set your wifi credentials
   "AutoConnectAP"

4. Netgotchi now lives in your network, it will scan periodically using pings and report potential breach of the honeypot
   Enjoy your Netgotchi !
   Please contribute to the project , let's save our network!

   Join our [reddit/r/Netgotchi](https://www.reddit.com/r/Netgotchi/)

 # License
 GNU General Public License v3.0 
 Created by MG ( MXZZ ) 


