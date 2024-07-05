# Netgotchi 
![immagine](https://github.com/MXZZ/Netgotchi/assets/3322271/947416e6-c088-4167-ba62-e69a6d1170ce)


reversed pwnagotchi : Netgotchi lives to protect your network 

A small arduino .ino script with some great functions to detect intruders or breach in the network, it pings periodically the network and report if any new device. It also has a rudimental "Honeypot" functionality with a rudimental exposed service, once someone reach the honeypot it will trigger an alarm. Please contribute to the repository via pull requests!

*Tested with nmap, service scan / intense scan will trigger the alert.* 


![immagine](https://github.com/MXZZ/Netgotchi/assets/3322271/cf8d7fec-7b33-4f14-9992-8cb4806633f2) ![immagine](https://github.com/MXZZ/Netgotchi/assets/3322271/68f4fe6c-9172-422b-ba39-ee901c098840)

Get an official build from [official shop *Ollestore* on Etsy](https://ollestore.etsy.com/listing/1742406142) and support the development! 



# Requirements 
- ESP8266 ( any )
- SSD1306 display ( ic version ) 
- Usb cable 
- optional 3d printed parts ( from thingiverse.com)

  or get an assembled one here on the [official shop *Ollestore* on Etsy](https://ollestore.etsy.com/listing/1742406142)

# Diagram 
![diagram_netgotchi](https://github.com/MXZZ/Netgotchi/assets/3322271/54fb9be5-4fe4-4ff3-b24a-f2a05287d893)


# Libraries
- It uses a modified version of ESP8266FtpServer ( replace your local one in Arduino>Libraries with this one)
- Adafruit_SSD1306 standard ( install from Library manager)
- ESPping Standard ( install from Library manager)
- WiFiManager standard ( install from Library manager)

# How to use
1. open .ino file , choose your subnet ( just change the value or add your own)
2. optionally you can input your Wifi credential in the .ino file or use the WifiManager library ( default is WifiManager) 

Current supported subnets ( or add your own) :
- 192.168.0.0/24 = type 0
- 192.168.1.0/24 = type 1
- 192.168.88.0/24 = type 2
- 192.168.100.0/24  = type 3

3. Install libraries and flash the file in the esp8266 ( connect the display first)
4. On first status WiFiManager library will create an hostpot where you can set your wifi credentials
   "AutoConnectAP"

5. Netgotchi now lives in your network, it will scan periodically using pings and report potential breach of the honeypot
   Enjoy your Netgotchi !
   Please contribute to the project , let's save our network!

   Join our [reddit/r/Netgotchi](https://www.reddit.com/r/Netgotchi/)

 # License
 GNU General Public License v3.0 
 Created by MG ( MXZZ ) 


