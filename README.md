# Netgotchi 
![immagine](https://github.com/MXZZ/Netgotchi/assets/3322271/947416e6-c088-4167-ba62-e69a6d1170ce)


reversed pwnagotchi : Netgotchi lives to protect your network 

A small arduino .ino script with some great functions to detect intruders or breach in the network, it pings periodically the network and report if any new device. It also has a rudimental "Honeypot" functionality with a rudimental exposed service, once someone reach the honeypot it will trigger an alarm. Please contribute to the repository via pull requests!

*Tested with nmap, service scan / intense scan will trigger the alert.* 


![immagine](https://github.com/MXZZ/Netgotchi/assets/3322271/cf8d7fec-7b33-4f14-9992-8cb4806633f2) ![immagine](https://github.com/MXZZ/Netgotchi/assets/3322271/68f4fe6c-9172-422b-ba39-ee901c098840)

Get an official build and support the development from [Netgotchi on Bonanza](https://www.bonanza.com/listings/NETGOTCHI-pwnagotchi-older-brother-scans-and-protect-your-network-/1657032899?fref=WCsgHJd3)



# Requirements 
- ESP8266 ( any )
- SSD1306 display ( ic version ) 
- Usb cable 
- optional 3d printed parts ( from thingiverse.com)
-    Community cases:  Wemos d1 https://www.printables.com/model/510481-terminal-for-ssd1306-096-oled-and-wemos-d1-mini ( thanks porkcube for the link)

  or get an assembled one here  : Get an official build and support the development from [Netgotchi on Bonanza](https://www.bonanza.com/listings/NETGOTCHI-pwnagotchi-older-brother-scans-and-protect-your-network-/1657032899?fref=WCsgHJd3)

# Diagram 
![diagram_netgotchi](https://github.com/MXZZ/Netgotchi/assets/3322271/54fb9be5-4fe4-4ff3-b24a-f2a05287d893)


# Libraries
- It uses a modified version of ESP8266FtpServer ( replace your local one in Arduino>Libraries with this one)
- Adafruit_SSD1306 standard ( install from Library manager)
- ESPping Standard ( install from Library manager)
- WiFiManager standard ( install from Library manager)

# How to use
1. ~~open .ino file , choose your subnet ( just change the value or add your own)~~ Not necessary due to Autodetect subnet function after v.0.7
2. optionally you can input your Wifi credential in the .ino file or use the WifiManager library ( default is WifiManager) 


3. Install libraries and flash the file in the esp8266 ( connect the display first)
4. On first status WiFiManager library will create an hostpot where you can set your wifi credentials
   "AutoConnectAP"

5. Netgotchi now lives in your network, it will scan periodically using pings and report potential breach of the honeypot
  


# Headless mode ( for Cyberdecks )
   ![immagine](https://github.com/MXZZ/Netgotchi/assets/3322271/f12ba979-5936-4bee-9d36-eba67ddebf59)
  - open script pyserial_cyberdeck and change line 5: /dev/ttyUSB0 to your COM port  ( on linux is usually /dev/ttyUSB0 ) 
  - run script utils/cyberdeck/pyserial_cyberdeck.py

# Scan for Vulnerabilities in the network ( v0.9 update ) 
  - Netgotchi now scan your network for services like quick-scan of NMAP - it will scan host for know vulnerables services including :
  { "Telnet", 23 },
  { "FTP", 21 },
  { "SSH", 22 },
  { "VNC", 5900 },
  { "RDP", 3389 },
  { "SMB", 445 },
  { "HTTP", 80 },
  { "HTTPS", 443 }
- report any vulnerabilities and vulnerable hosts with a "WRNG!"
- *This may trigger other Netgotchi in the same network* , so there is a flag enabled by default to skip the FTP scan.
- can be disabled from the *securityScanActive* flag

# Suppor the project
 Enjoy your Netgotchi !
   Please contribute to the project,and let's save our network!
  Thank you to all contributors and supporters! (^///^)9 

   Join our [reddit/r/Netgotchi](https://www.reddit.com/r/Netgotchi/)

 # License
 GNU General Public License v3.0 
 Created by MG ( MXZZ ) 


