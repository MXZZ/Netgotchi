# ESP32_FTPServer

FTP Server for ESP32 (ESP32-CAM)
 
based on FTP Serveur for Arduino Due and Ethernet shield (W5100) or WIZ820io (W5200)  
based on Jean-Michel Gallego's work  
based on David Paiva's work  

01/01/2020  Modified by Ed Williams to support active or passive modes. No wildcards are supported.   
  Works with Windows FTP, UNIX FTP, WinSCP, Classic FTP and Firefox FTP clients that I have access to.  
  It's definitely not complete but works well enough for me.  

FTP_Server_Example.ino is an example of running this FTP Server on an ESP32-CAM.  The essential  
steps to use are:  

1.  Include ESP32FtpServer.cpp and ESP32FtpServer.h with your sketch.

2.  Include ESP32FtpServer.h in your ino file and declare an FTPServer: 

      #include "ESP32FtpServer.h"  
      FTPServer ftpSrv;  

3.  In setup():
    - Start a WiFi connection
    - Mount the SD Card
    - Start the FTP Server giving a username and password:

      ftpSrv.begin("espuser", "esppass");

4.  In loop() include a frequent call to the FTPServer handler:

      void loop()  
      {  
        ftpSrv.handleFTP();  
        delay(10);  
      }  
      

