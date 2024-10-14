# 🛡️ Netgotchi

![Netgotchi Image](https://github.com/MXZZ/Netgotchi/assets/3322271/947416e6-c088-4167-ba62-e69a6d1170ce)

> Netgotchi: Your network's loyal guardian! 🐾

A small arduino .ino script with some great functions to detect intruders or breaches in the network, it pings periodically the network and reports if any new device. It also has a rudimental "Honeypot" functionality with a rudimental exposed service, once someone reaches the honeypot it will trigger an alarm. Please contribute to the repository via pull requests!

Tested with Nmap, service scan / intense scan will trigger the alert.

## 🌟 Features

- 🔍 Periodic network scanning
- 🍯 Built-in "Honeypot" functionality
- 🚨 Intrusion detection and alerting
- 🖥️ Support for multiple OLED display types
- 🔄 Supports both ESP8266 and ESP32

![Netgotchi Screen 1](https://github.com/MXZZ/Netgotchi/assets/3322271/cf8d7fec-7b33-4f14-9992-8cb4806633f2) ![Netgotchi Screen 2](https://github.com/MXZZ/Netgotchi/assets/3322271/68f4fe6c-9172-422b-ba39-ee901c098840)

## 🛒 Get an Official Build and support the development ❤ 

Support the development by getting an official Netgotchi build:

- [Tindie](https://www.tindie.com/products/ollestore/negotchi-network-security-scanner/)
- [Etsy](https://olleadventures.etsy.com/listing/1752764124)
- [Netgotchi on Bonanza](https://www.bonanza.com/listings/NETGOTCHI-pwnagotchi-older-brother-scans-and-protect-your-network-/1657032899?fref=WCsgHJd3)
- [Amazon](https://amzn.to/4dd5yjA)

### Note : for security purpose make sure you buying from from official the link above, you will receive the latest version of this repository, without any modification.  

PRO version & Prototypes 


![IMG_20240829_1519522](https://github.com/user-attachments/assets/b584b5f6-9727-46fc-9bce-9cad7c8529e4)

Netgotchi Pro version :
Pro version has a keypad to access the settings, change mode like chat-device ( TEXTgotchi )  or gpio remote controller ( CTRLgotchi ) , from the device itself without re-flashing settings and a small buzzer for sound alerts!
- [Netgotchi PRO on Tindie](https://www.tindie.com/products/35655/)
- [Netgotchi PRO on Etsy](https://olleadventures.etsy.com/listing/1771783598)
- [Netgotchi PRO on Amazon](https://amzn.to/4gfUogy) 


## 🛠️ Requirements

- ESP8266 or ESP32
- OLED display (SSD1306, SH1106, or SSD1305)
- USB cable
- Optional: 3D printed case ([Community case for Wemos D1](https://www.printables.com/model/510481-terminal-for-ssd1306-096-oled-and-wemos-d1-mini))

## 📊 Wiring Diagram
ESP8266
![Netgotchi Diagram](https://github.com/MXZZ/Netgotchi/assets/3322271/54fb9be5-4fe4-4ff3-b24a-f2a05287d893)
ESP32
![esp32](https://github.com/user-attachments/assets/cc486dfd-fdb6-468b-a158-2e0a78891ac4)


## 📚 Libraries

- ESP8266/ESP32 core libraries
- Modified FTP Server library (ESP8266FtpServer or ESP32FtpServer) -- you need to install the one provided in the "/libraries" folder in this repo
- Adafruit_GFX
- Adafruit_SSD1306, Adafruit_SH110X, or Adafruit_SSD1305 (based on your OLED type)
- ESPping
- NTPClient
- WiFiManager
- Button2

## 🚀 How to Use

1. Open the `.ino` file in the Arduino IDE, make sure you have all files open like the image below :
   ![immagine](https://github.com/user-attachments/assets/552f5d19-d55d-4d47-9ef4-f200438421e6)

2. Select your OLED display type by setting the appropriate flag to 1 (e.g., `#define oled_type_ssd1306 1`).![immagine](https://github.com/user-attachments/assets/c1fef59b-e22a-4555-94cb-8ef26b71e756)

3. Install the required libraries via the Arduino Library Manager.
4. Select your board (ESP8266 or ESP32) in the Arduino IDE.
5. Flash the code to your board.
6. On the first boot, Netgotchi will create a WiFi hotspot named "AutoConnectAP" for you to set up your WiFi credentials.
7. Once connected, Netgotchi will start guarding your network!

Netgotchi Pro configuration :
 
![immagine](https://github.com/user-attachments/assets/8470aba4-9c47-469d-80bb-6da349b01436)


## 🖥️ Headless Mode (for Cyberdecks)

![Headless Mode](https://github.com/MXZZ/Netgotchi/assets/3322271/f12ba979-5936-4bee-9d36-eba67ddebf59)

1. Open `utils/cyberdeck/pyserial_cyberdeck.py`
2. Change the COM port on line 5 (e.g., `/dev/ttyUSB0` for Linux, `COMx` for Windows)
3. Run the script

## 🔎 Vulnerability Scanning

Netgotchi scans for potentially vulnerable services including:
- Telnet (23)
- FTP (21)
- SSH (22)
- VNC (5900)
- RDP (3389)
- SMB (445)
- HTTP (80)
- HTTPS (443)

Vulnerable hosts are marked with "WRNG!". This feature can be toggled with the `securityScanActive` flag.

## 🖥️ Web Interface ( v.1.4 update)
Visit your netgotchi ip address at the port :80 ( the IP will be visible on the screen once connected )
![netgotchi222](https://github.com/user-attachments/assets/e5473956-e858-45dd-8c93-b7030d33723e)

## How to flash ( webmode - easy mode ) 
- Connect your ESP8266 or ESP32 or Netgotchi via USB cable ( Data cable)
- Use Chrome to go https://espressif.github.io/esptool-js/ 
- Press "Connect" 
- Press "Add File" , select Netgotchi.ino.bin ( Find it in "Release")
- Press "Flash" 
- Restart your Netgotchi , Enjoy!

 
## 😊 Join us on Discord

Discord Server: [Join Now](https://discord.gg/hM4w8eTKrt)!

## 🤝 Contributing

We welcome contributions! Please submit your pull requests to help make Netgotchi even better.

Join our [Reddit community](https://www.reddit.com/r/Netgotchi/)!

## 📜 License

GNU General Public License v3.0

Created with ❤️ by MG [MXZZ](https://github.com/MXZZ) | ESP32 Port Created by [itsOwen](https://github.com/itsOwen)
