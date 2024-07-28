# 🛡️ Netgotchi

![Netgotchi Image](https://github.com/MXZZ/Netgotchi/assets/3322271/947416e6-c088-4167-ba62-e69a6d1170ce)

> Netgotchi: Your network's loyal guardian! 🐾

A small arduino .ino script with some great functions to detect intruders or breach in the network, it pings periodically the network and report if any new device. It also has a rudimental "Honeypot" functionality with a rudimental exposed service, once someone reach the honeypot it will trigger an alarm. Please contribute to the repository via pull requests!

Tested with nmap, service scan / intense scan will trigger the alert.

## 🌟 Features

- 🔍 Periodic network scanning
- 🍯 Built-in "Honeypot" functionality
- 🚨 Intrusion detection and alerting
- 🔄 Supports both ESP8266 and ESP32

![Netgotchi Screen 1](https://github.com/MXZZ/Netgotchi/assets/3322271/cf8d7fec-7b33-4f14-9992-8cb4806633f2) ![Netgotchi Screen 2](https://github.com/MXZZ/Netgotchi/assets/3322271/68f4fe6c-9172-422b-ba39-ee901c098840)

## 🛒 Get an Official Build

Support the development by getting an official Netgotchi:
- [Netgotchi on Bonanza](https://www.bonanza.com/listings/NETGOTCHI-pwnagotchi-older-brother-scans-and-protect-your-network-/1657032899?fref=WCsgHJd3)
- [Amazon](https://amzn.to/4dd5yjA)
- [Etsy](https://olleadventures.etsy.com/listing/1752764124)

## 🛠️ Requirements

- ESP8266 or ESP32
- SSD1306 OLED display (I2C version)
- USB cable
- Optional: 3D printed case ([Community case for Wemos D1](https://www.printables.com/model/510481-terminal-for-ssd1306-096-oled-and-wemos-d1-mini))

## 📊 Wiring Diagram

![Netgotchi Diagram](https://github.com/MXZZ/Netgotchi/assets/3322271/54fb9be5-4fe4-4ff3-b24a-f2a05287d893)

## 📚 Libraries

- Modified ESP8266FtpServer (for ESP8266) or ESP32FtpServer (for ESP32)
- Adafruit_SSD1306
- ESPping
- WiFiManager

## 🚀 How to Use

1. Open the `.ino` file and select your board type (ESP8266 or ESP32) by commenting/uncommenting the appropriate `#define` at the top of the file.
2. Install the required libraries via the Arduino Library Manager.
3. Flash the code to your ESP8266 or ESP32.
4. On first boot, Netgotchi will create a WiFi hotspot named "AutoConnectAP" for you to set up your WiFi credentials.
5. Once connected, Netgotchi will start guarding your network!

## 🖥️ Headless Mode (for Cyberdecks)

![Headless Mode](https://github.com/MXZZ/Netgotchi/assets/3322271/f12ba979-5936-4bee-9d36-eba67ddebf59)

1. Open `utils/cyberdeck/pyserial_cyberdeck.py`
2. Change the COM port on line 5 (e.g., `/dev/ttyUSB0` for Linux, `COMx` for Windows)
3. Run the script

## 🔎 Vulnerability Scanning

As of v0.9, Netgotchi scans for potentially vulnerable services including:
- Telnet (23)
- FTP (21)
- SSH (22)
- VNC (5900)
- RDP (3389)
- SMB (445)
- HTTP (80)
- HTTPS (443)

Vulnerable hosts are marked with "WRNG!". This feature can be toggled with the `securityScanActive` flag.

## 😊 Join us on Discord

Discord Server: [Join Now](https://discord.gg/qucvvgkv3B)

## 🤝 Contributing

We welcome contributions! Please submit your pull requests to help make Netgotchi even better.

Join our [Reddit community](https://www.reddit.com/r/Netgotchi/)!

## 📜 License

GNU General Public License v3.0

Created with ❤️ by MG [MXZZ](https://github.com/MXZZ) | ESP32 Port Created by [itsOwen](https://github.com/itsOwen)