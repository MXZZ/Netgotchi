/*
 * FTP Server for ESP32
 * 
 * based on FTP Serveur for Arduino Due and Ethernet shield (W5100) or WIZ820io (W5200)
 * based on Jean-Michel Gallego's work
 * modified to work with esp8266 SPIFFS by David Paiva david@nailbuster.com
 * 
 * Modified by Ed Williams to support active or passive modes. No wildcards are supported. 
 *   Works with Windows FTP, UNIX FTP, WinSCP, Classic FTP and Firefox FTP clients that I 
 *   have access to. It's definitely not complete but works well enough for me.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * 2017: modified by @robo8080
 * 2019: modified by @fa1ke5
 * 2020-01-01: Modified by @mtnbkr88 (Ed Williams)
 * 
 */

#include "ESP32FtpServer.h"

#include <WiFi.h>
#include <FS.h>
#include "SD_MMC.h"

#include <string.h> 
using namespace std; 

WiFiServer ftpServer( FTP_CTRL_PORT );
WiFiServer dataServer( FTP_DATA_PORT );

void FtpServer::begin(String uname, String pword)
{
  // Tells the ftp server to begin listening for incoming connection
  _FTP_USER = uname;
  _FTP_PASS = pword;

  // Default for data port
  dataPort = FTP_DATA_PORT;
  
  ftpServer.begin();
  delay(10);
  dataServer.begin( FTP_DATA_PORT ); 
  delay(10);
  millisTimeOut = (uint32_t)FTP_TIME_OUT * 60 * 1000;
  millisDelay = 0;
  cmdStatus = 0;
    iniVariables();
}

void FtpServer::iniVariables()
{
  // Default Data connection is Active
  dataPassiveConn = false;
  
  // Set the root directory
  strcpy( cwdName, "/" );

  rnfrCmd = false;
  transferStatus = 0;
  
}

void FtpServer::handleFTP()
{
  if((int32_t) ( millisDelay - millis() ) > 0 )
    return;

  if (ftpServer.hasClient()) { // returns true when a remote computer is trying to connect
    client.stop();  // dump old client connection if there was one
    client = ftpServer.available();  // point client to the new remote connection
  }
  
  if( cmdStatus == 0 ) // if command status is zero, then server is not ready to handle it
  {
    if( client.connected()) 
      disconnectClient(); // if connection not expected, close it
    cmdStatus = 1;
  }
  else if( cmdStatus == 1 )         // Ftp server waiting for connection
  {
    abortTransfer();  // killing all current activity and setting up for new connections
    iniVariables();
    #ifdef FTP_DEBUG
      Serial.println("Ftp server waiting for connection on port "+ String(FTP_CTRL_PORT));
    #endif
    cmdStatus = 2;
  }
  else if( cmdStatus == 2 )         // Ftp server idle
  {
    if( client.connected() )        // true if a client is currently connected
    {
      clientConnected();      
      millisEndConnection = millis() + 10 * 1000 ; // wait client id during 10 s.
      cmdStatus = 3;
    }
  }
  else if( readChar() > 0 )         // got response
  {
    if( ! strcmp( command, "OPTS" ) || ( ! strcmp( command, "SYST" )) || ( ! strcmp( command, "FEAT" )) ) {
        processCommand(); // process OPTS, SYST and FEAT
      }
    else if( cmdStatus == 3 )       // Ftp server waiting for user identity
      { if( userIdentity() )
          cmdStatus = 4;
        else
          cmdStatus = 0;
      }        
    else if( cmdStatus == 4 )       // Ftp server waiting for user registration
      if( userPassword() )
      {
        cmdStatus = 5;
        millisEndConnection = millis() + millisTimeOut;
      }
      else
        cmdStatus = 0;
    else if( cmdStatus == 5 )       // Ftp server waiting for user command
      if( ! processCommand())
        cmdStatus = 0;
      else
        millisEndConnection = millis() + millisTimeOut;
  }
  else if (!client.connected() || !client)
  {
    cmdStatus = 1;
      #ifdef FTP_DEBUG
      Serial.println("client disconnected");
    #endif
  }

  if( transferStatus == 1 )         // Retrieve data
  {
    if( ! doRetrieve())
      transferStatus = 0;
  }
  else if( transferStatus == 2 )    // Store data
  {
    if( ! doStore())
      transferStatus = 0;
  }
  else if( cmdStatus > 2 && ! ((int32_t) ( millisEndConnection - millis() ) > 0 ))
  {
    client.print("530 Timeout\r\n");
    millisDelay = millis() + 200;    // delay of 200 ms
    cmdStatus = 0;
  }
}

void FtpServer::clientConnected()
{
  #ifdef FTP_DEBUG
    Serial.println("Client connected!");
  #endif
  client.print( "220 Welcome to FTP for ESP32 Version "+ String(FTP_SERVER_VERSION) +"\r\n");
  iCL = 0;
}

void FtpServer::disconnectClient()
{
  #ifdef FTP_DEBUG
    Serial.println(" Disconnecting client");
  #endif
  abortTransfer();
  client.print("221 Goodbye\r\n");
  client.stop();
}

boolean FtpServer::userIdentity()
{ 
  if( strcmp( command, "USER" ))             
    client.print( "500 Syntax error\r\n");
  if( strcmp( parameters, _FTP_USER.c_str() ))
    client.print( "530 user not found\r\n");
  else
  {
    client.print( "331 OK. Password required\r\n");
    strcpy( cwdName, "/" );
    return true;
  }
  millisDelay = millis() + 100;  // delay of 100 ms
  return false;
}

boolean FtpServer::userPassword()
{
  if( strcmp( command, "PASS" ))
    client.print( "500 Syntax error\r\n");
  else if( strcmp( parameters, _FTP_PASS.c_str() ))
    client.print( "530 Invalid password\r\n");
  else
  {
    #ifdef FTP_DEBUG
      Serial.println( "OK. Waiting for commands.");
    #endif
    client.print( "230 Login successful.\r\n");
    return true;
  }
  millisDelay = millis() + 100;  // delay of 100 ms
  return false;
}

boolean FtpServer::processCommand()
{
  ///////////////////////////////////////
  //                                   //
  //      ACCESS CONTROL COMMANDS      //
  //                                   //
  ///////////////////////////////////////

  //
  //  CDUP - Change to Parent Directory 
  //
  if( ! strcmp( command, "CDUP" ) || ( ! strcmp( command, "CWD" ) && ! strcmp( parameters, ".." )))
  {
   bool ok = false;
   if( strlen( cwdName ) > 1 )            // do nothing if cwdName is root
    {
      // if cwdName ends with '/', remove it (must not append)
      if( cwdName[ strlen( cwdName ) - 1 ] == '/' )
        cwdName[ strlen( cwdName ) - 1 ] = 0;
      // search last '/'
      char * pSep = strrchr( cwdName, '/' );
      ok = pSep > cwdName;
      // if found, ends the string on its position
      if( ok )
      {
        * pSep = 0;
        ok = SD_MMC.exists( cwdName );
      }
    }
    // if an error appends, move to root
    if( ! ok )
      strcpy( cwdName, "/" );
   
    client.print("250 Ok. Current directory is " + String(cwdName) + "\r\n");
  }

  //
  //  CWD - Change Working Directory
  //
  else if( ! strcmp( command, "CWD" ) || ! strcmp( command, "XCWD" ) )
  { 
    // check if first three of new path is "/.."
    char first3[4];
    first3[0] = parameters[0];
    first3[1] = parameters[1];
    first3[2] = parameters[2];
    if ( !strcmp( first3, "/.." ) ) { // if yes, remove "/.." from head of parameters
      int pl = strlen(parameters)-3;
      for (int i=0; i < pl; i++)
        parameters[i] = parameters[i+3];   
      parameters[pl] = 0;
    }
    char path[ FTP_CWD_SIZE ];
    char curcwdName[ FTP_CWD_SIZE ];
    strcpy( curcwdName, cwdName);
    if( haveParameter() && makeExistsPath( path ))
    {
      strcpy( cwdName, path );
      File dir = SD_MMC.open(cwdName);
      if(dir.isDirectory())
        client.print( "250 Ok. Current directory is " + String(cwdName) + "\r\n");
      else {
        client.print( "550 " + String(cwdName) + " is not a directory\r\n");
        strcpy(cwdName, curcwdName);
      }
    }
  }

  //
  //  PWD - Print Directory
  //
  else if( ! strcmp( command, "PWD" ) || ! strcmp( command, "XPWD" ) )
    client.print( "257 \"" + String(cwdName) + "\" is your current directory\r\n");

  //
  //  QUIT
  //
  else if( ! strcmp( command, "QUIT" ))
  {
    disconnectClient();
    return false;
  }

  ///////////////////////////////////////
  //                                   //
  //    TRANSFER PARAMETER COMMANDS    //
  //                                   //
  ///////////////////////////////////////

  //
  //  MODE - Transfer Mode 
  //
  else if( ! strcmp( command, "MODE" ))
  {
    if( ! strcmp( parameters, "S" ))
      client.print( "200 S Ok\r\n");
    else
      client.print( "504 Only S(tream) is supported\r\n");
  }

  //
  //  PASV - Passive Connection management
  //
  else if( ! strcmp( command, "PASV" ))
  {
    if (data.connected()) data.stop(); // drop old data connection if connected
    if (dataPort != FTP_DATA_PORT_PASV) { // set to passive port if not already set
      //dataIp = Ethernet.localIP();    
      dataIp = WiFi.localIP();  
      dataPort = FTP_DATA_PORT_PASV;
      dataServer.stop();
      delay(10);
      dataServer.begin( FTP_DATA_PORT_PASV );
      delay(10);
    }
    #ifdef FTP_DEBUG
      Serial.println("Connection management set to passive");
      Serial.println( "Local data port set to " + String(dataPort));
    #endif
    client.print( "227 Entering Passive Mode ("+ String(dataIp[0]) + "," + String(dataIp[1])+","+ String(dataIp[2])+","+ String(dataIp[3])+","+String( dataPort >> 8 ) +","+String ( dataPort & 255 )+").\r\n");
    dataPassiveConn = true;
  }

  //
  //  PORT - Data Port
  //
  else if( ! strcmp( command, "PORT" ))
  {
    if (data) data.stop();  // drop any current data connection
                            // to setup for a new active data connection 
    // get IP of data client
    dataIp[ 0 ] = atoi( parameters );
    char * p = strchr( parameters, ',' );
    for( uint8_t i = 1; i < 4; i ++ )
    {
      dataIp[ i ] = atoi( ++ p );
      p = strchr( p, ',' );
    }
    // get port of data client
    dataPort = 256 * atoi( ++ p );
    p = strchr( p, ',' );
    dataPort += atoi( ++ p );
    if( p == NULL )
      client.print( "501 Can't interpret parameters\r\n");
    else
    {
      dataServer.stop();
      delay(10);
      dataServer.begin( FTP_DATA_PORT );
      delay(10);
      #ifdef FTP_DEBUG
        Serial.println("Connection management set to active");
        Serial.println( "Local data port set to " + String( FTP_DATA_PORT ));
      #endif
      client.print("200 PORT command successful\r\n");
      dataPassiveConn = false;
    }
  }

  //
  //  STRU - File Structure
  //
  else if( ! strcmp( command, "STRU" ))
  {
    if( ! strcmp( parameters, "F" ))
      client.print( "200 F Ok\r\n");
    else
      client.print( "504 Only F(ile) is suported\r\n");
  }

  //
  //  TYPE - Data Type
  //
  else if( ! strcmp( command, "TYPE" ))
  {
    if( ! strcmp( parameters, "A" ))
      client.print( "200 Switching to ASCII mode\r\n");
    else if( ! strcmp( parameters, "I" ))
      client.print( "200 Switching to Binary mode\r\n");
    else
      client.print( "504 Unknown TYPE\r\n");
  }

  ///////////////////////////////////////
  //                                   //
  //        FTP SERVICE COMMANDS       //
  //                                   //
  ///////////////////////////////////////

  //
  //  ABOR - Abort
  //
  else if( ! strcmp( command, "ABOR" ))
  {
    abortTransfer();
    client.print( "226 Data connection closed\r\n");
  }

  //
  //  DELE - Delete a File 
  //
  else if( ! strcmp( command, "DELE" ))
  {
    char path[ FTP_CWD_SIZE ];
    if( strlen( parameters ) == 0 )
      client.print( "501 No file name\r\n");
    else if( makePath( path ))
    {
      if( ! SD_MMC.exists( path ))
        client.print( "550 File " + String(parameters) + " not found\r\n");
      else
      {
        if( SD_MMC.remove( path ))
          client.print( "250 Deleted " + String(parameters) + "\r\n" );
        else
          client.print( "450 Can't delete " + String(parameters) + "\r\n" );
      }
    }
  }
  
  //
  //  LIST - List 
  //
  else if( ! strcmp( command, "LIST" ))
    {
    if(dataConnect()){

      char path[ FTP_CWD_SIZE ] = "";
      char buffer[256] = "";
      
      if ( ! makePath( path )) {
        client.print( "550 File/Directory " +String(parameters)+ " not found\r\n");
      }
      else {
        File dir = SD_MMC.open(path);
        if ( !dir ) 
          client.print( "550 Can't open " + String(path) + "\r\n" );
        else {
          uint16_t nm = 0;
          if (dir.isDirectory()) { // process a directory listing 
            client.print( "150 Here comes the directory listing\r\n");
            File file = dir.openNextFile();
            while( file == 1) {
              if ( formatLIST ( &file, buffer ) ) {
                data.print( String(buffer) + "\r\n" );
                #ifdef FTP_DEBUG
                  Serial.println( String(buffer) );
                #endif      
                nm ++;
              }
              file = dir.openNextFile();
              if (file < 1 ) {
                break;
              }
            }
            client.print( "226 Directory send OK.\r\n");
            Serial.println( "226 " + String(nm) + " matches total");
            }
          else { // process a file listing
            client.print( "150 Here comes the file listing\r\n");
            if ( formatLIST ( &dir, buffer ) ) {
              data.print( String(buffer) + "\r\n" );
              #ifdef FTP_DEBUG
                Serial.println( String(buffer) );
              #endif      
              nm ++;
              client.print( "226 File send OK.\r\n");
              Serial.println( "226 " + String(nm) + " matches total");
            }
            else {
              client.print( "550 Can't open " + String(path) + "\r\n" );
            }
          }
        }
      }
      data.stop();
    }
    else {
      client.print( "425 No data connection\r\n");
    }
  }
    
  //
  //  MLSD - Listing for Machine Processing (see RFC 3659)
  //
  else if( ! strcmp( command, "MLSD" ))
    {
    if( dataConnect() ){

      char path[ FTP_CWD_SIZE ];
      char buffer[256] = "";

      if ( ! makePath( path )) {
        client.print( "550 Directory " +String(parameters)+ " not found\r\n");
      }
      else {
        File dir = SD_MMC.open(path);
        if ( !dir ) 
          client.print( "550 Can't open " + String(path) + "\r\n" );
        else {
          uint16_t nm = 0;
          if (!dir.isDirectory())  // don't process a name file 
            client.print( "501 " + String(path) + " is not a directory\r\n");
          else {
            client.print( "150 Here comes the directory listing\r\n");
            File file = dir.openNextFile();
            while( file == 1) {
              if ( formatMLST ( &file, buffer ) ) {
                #ifdef FTP_DEBUG
                  Serial.println( String(buffer) );
                #endif  
                data.print( String(buffer) + "\r\n" );
                nm ++;
              }
              file = dir.openNextFile();
              if (file < 1 ) {
                break;
              }
            }
            client.print( "226 Directory send OK.\r\n");
            Serial.println( "226 " + String(nm) + " matches total");
          }
        }
      }
      data.stop();
    }
    else {
      client.print( "425 No data connection\r\n");
    }
  }
    
  //
  //  MLST - Single Listing for Machine Processing (see RFC 3659)
  //
  else if( ! strcmp( command, "MLST" ))
    {
      char path[ FTP_CWD_SIZE ];
      char buffer[256] = "";

      if ( ! makePath( path )) {
        client.print( "550 Path " +String(parameters)+ " not found\r\n");
      }
      else {
        File dir = SD_MMC.open(path);
        if ( !dir ) 
          client.print( "550 Can't open " + String(path) + "\r\n" );
        else {
          uint16_t nm = 0;
          client.print( "250- Listing " + String(path) + "\r\n");
          formatMLST ( &dir, buffer );
          char * p = strchr( buffer, ' ' );
          buffer[p-buffer+1] = 0;
          strncat( buffer, path, 256 );
            #ifdef FTP_DEBUG
              Serial.println( String(buffer) );
            #endif  
          client.print( " " + String(buffer) + "\r\n" );
          client.print( "250 End\r\n");
        }
      }
    }
    
  //
  //  NLST - Name List      
  //
  else if( ! strcmp( command, "NLST" ))
  {
    if( ! dataConnect())
      client.print( "425 No data connection\r\n");
    else
    {
      char path[ FTP_CWD_SIZE ] = "";
      
      if ( ! makePath( path )) 
        client.print( "550 Directory " +String(parameters)+ " not found\r\n");
      else {
        File dir = SD_MMC.open(path);
        if ( !dir ) 
          client.print( "550 Can't open directory " + String(path) + "\r\n" );
        else {
          if ( ! dir.isDirectory() )  // process a directory listing 
            client.print( "550 Can't open directory " + String(path) + "\r\n" );
          else {
            uint16_t nm = 0;
            client.print( "150 Here comes the directory listing\r\n");

            File file = dir.openNextFile();
            while( file) {
              if ( ! file.isDirectory()) { 
                data.print( String(file.name()) + "\r\n");
                nm ++;
              }
              file = dir.openNextFile();
            }
            client.print( "226 " + String(nm) + " matches total\r\n");
          }
        }
      }
      data.stop();
    }
  }

  //
  //  NOOP
  //
  else if( ! strcmp( command, "NOOP" ))
  {
    // dataPort = 0;
    client.print( "200 Zzz...\r\n");
  }

  //
  //  RETR - Retrieve
  //
  else if( ! strcmp( command, "RETR" ))
  {
    char path[ FTP_CWD_SIZE ];
    if( strlen( parameters ) == 0 )
      client.print( "501 No file name\r\n");
    else if( makePath( path ))
    {
    file = SD_MMC.open(path, "r");
      if( !file) {
        client.print( "550 File " +String(parameters)+ " not found\r\n");
        Serial.println("550");
      }
      else if( ! dataConnect()) {
        client.print( "425 No data connection\r\n");
        Serial.println("425");
      }
      else
      {
        #ifdef FTP_DEBUG
          Serial.println("Sending " + String(parameters));
        #endif
        client.print( "150-Connected to port "+ String(dataPort) + "\r\n150 " + String(file.size()) + " bytes to download\r\n");
        millisBeginTrans = millis();
        bytesTransfered = 0;
        transferStatus = 1;
      }
    }
  }

  //
  //  STOR - Store
  //
  else if( ! strcmp( command, "STOR" ))
  {
    char path[ FTP_CWD_SIZE ];
    if( strlen( parameters ) == 0 )
      client.print( "501 No file name\r\n");
    else if( makePath( path ))
    {
    file = SD_MMC.open(path, "w");
      if( !file)
        client.print( "451 Can't open/create " + String(parameters) + "\r\n" );
      else if( ! dataConnect())
      {
        client.print( "425 No data connection\r\n");
        file.close();
      }
      else
      {
        #ifdef FTP_DEBUG
          Serial.println( "Receiving " +String(parameters));
        #endif
        client.print( "150 Connected to port " + String(dataPort) + "\r\n" );
        millisBeginTrans = millis();
        bytesTransfered = 0;
        transferStatus = 2;
      }
    }
  }

  //
  //  MKD - Make Directory
  //
  
  else if( ! strcmp( command, "MKD" ) || ! strcmp( command, "XMKD" ))
  {
     char path[ FTP_CWD_SIZE ];
     if( haveParameter() && makePath( path )){
      if (SD_MMC.exists( path )){
        client.print( "521 Can't create \"" + String(parameters) + ", Directory exists\r\n");
        }
        else
        {
          if( SD_MMC.mkdir( path )){
            client.print( "257 \"" + String(parameters) + "\" created\r\n");
          }
          else{
            client.print( "550 Can't create \"" + String(parameters) + "\r\n" );
          }
        }
      }
  }

  //
  //  RMD - Remove a Directory 
  //
  else if( ! strcmp( command, "RMD" ) || ! strcmp( command, "XRMD" ))
  {
   char path[ FTP_CWD_SIZE ];
     if( haveParameter() && makePath( path )){
      if( SD_MMC.rmdir( path )){
        #ifdef FTP_DEBUG
          Serial.println( " Deleting " +String(parameters));
        #endif
        client.print( "250 \"" + String(parameters) + "\" deleted\r\n");
      }
      else
      {
        client.print( "550 Can't remove \"" + String(parameters) + "\". Directory not found or not empty\r\n");  
      }
    } 
  }

  //
  //  RNFR - Rename From 
  //
  else if( ! strcmp( command, "RNFR" ))
  {
    buf[ 0 ] = 0;
    if( strlen( parameters ) == 0 )
      client.print( "501 No file name\r\n");
    else if( makePath( buf ))
    {
      if( ! SD_MMC.exists( buf ))
        client.print( "550 File " +String(parameters)+ " not found\r\n");
      else
      {
        #ifdef FTP_DEBUG
          Serial.println("Renaming " + String(buf));
        #endif
        client.print( "350 RNFR accepted - file exists, ready for destination\r\n");     
        rnfrCmd = true;
      }
    }
  }

  //
  //  RNTO - Rename To 
  //
  else if( ! strcmp( command, "RNTO" ))
  {  
    char path[ FTP_CWD_SIZE ];
    char dir[ FTP_FIL_SIZE ];
    if( strlen( buf ) == 0 || ! rnfrCmd )
      client.print( "503 Need RNFR before RNTO\r\n");
    else if( strlen( parameters ) == 0 )
      client.print( "501 No file name\r\n");
    else if( makePath( path ))
    {
      if( SD_MMC.exists( path ))
        client.print( "553 " +String(parameters)+ " already exists\r\n");
      else
      {          
        #ifdef FTP_DEBUG
          Serial.println("Renaming " + String(buf) + " to " + String(path));
        #endif
        if( SD_MMC.rename( buf, path ))
          client.print( "250 File successfully renamed or moved\r\n");
        else
          client.print( "451 Rename/move failure\r\n");
      }
    }
    rnfrCmd = false;
  }

  //
  //  SYST - System Type
  //
  else if( ! strcmp( command, "SYST" ))
  {
      client.print( "215 UNIX Type: L8\r\n");
  }

  ///////////////////////////////////////
  //                                   //
  //   EXTENSIONS COMMANDS (RFC 3659)  //
  //                                   //
  ///////////////////////////////////////

  //
  //  FEAT - New Features
  //
  else if( ! strcmp( command, "FEAT" ))
  {
    client.print( "211-Features:\r\n MDTM\r\n MLSD\r\n MLST\r\n SIZE\r\n211 END\r\n" );
  }

  //
  //  MDTM - File Modification Time (see RFC 3659)
  //
  else if (!strcmp(command, "MDTM"))
  {
    char path[ FTP_CWD_SIZE ];
    if( strlen( parameters ) == 0 )
      client.print( "550 No file name\r\n");
    else if( makePath( path ))
  {
    file = SD_MMC.open(path, "r");
      if(!file)
         client.print( "550 Can't open " +String(parameters) + "\r\n" );
      else
      {
        char ft[21];
        time_t ftime = file.getLastWrite();
        strftime(ft, sizeof(ft), "%Y%m%d%H%M%S", gmtime(&ftime));
        #ifdef FTP_DEBUG
          Serial.println("MDTM " + String(ft));
        #endif
        client.print( "213 " + String(ft) + "\r\n" );
        file.close();
      }
    }
  }

  //
  //  SIZE - Size of the file
  //
  else if( ! strcmp( command, "SIZE" ))
  {
    char path[ FTP_CWD_SIZE ];
    if( strlen( parameters ) == 0 )
      client.print( "550 No file name\r\n");
    else if( makePath( path ))
  {
    file = SD_MMC.open(path, "r");
      if(!file)
         client.print( "550 Can't open " +String(parameters) + "\r\n" );
      else
      {
        #ifdef FTP_DEBUG
          Serial.println("SIZE " + String(file.size()));
        #endif
        client.print( "213 " + String(file.size()) + "\r\n" );
        file.close();
      }
    }
  }

  //
  //  SITE - System command
  //
  else if( ! strcmp( command, "SITE" ))
  {
      client.print( "500 Unknown SITE command " + String(parameters) + "\r\n" );
  }

  //
  //  OPTS - OPTS UTF8 ON
  //
  else if( ! strcmp( command, "OPTS" ))
  {
      client.print( "200 None\r\n");
  }

  //
  //  Unrecognized commands ...
  //
  else
    client.print( "500 Unknown command\r\n");
  
  return true;
}

boolean FtpServer::dataConnect()
{
  unsigned long startTime = millis();
  //wait 5 seconds for a data connection
  if (!dataPassiveConn) data.connect(dataIp, dataPort);  // connect to client on data port
                                                         // when in active mode
  else
    {
    while (!dataServer.hasClient() && millis() - startTime < 10000)
      // wait for a data connection
      {
      yield(); // don't hog the cpu
      if (dataServer.hasClient()){
        Serial.println("Found data connection");
        break;
        }
      #ifdef FTP_DEBUG
      else {
        Serial.println("Waiting for data connection...");
        }
      #endif
      }  // end of waiting for for connection loop
    if (dataServer.hasClient()) { // if dataServer has a connection...
      data = dataServer.available();  // connect new dataServer connection to client data connection
      #ifdef FTP_DEBUG
          Serial.println("ftpdataserver client connected...");
      #endif
    
    }
  }
  return data.connected();  // return true if client data is connected
}

boolean FtpServer::doRetrieve()
{
  if (data.connected()) {
    int16_t nb = file.readBytes(buf, FTP_BUF_SIZE);
    if (nb > 0) {
      data.write((uint8_t*)buf, nb);
      bytesTransfered += nb;
      return true;
    }
  }
  closeTransfer();
  return false;
}

boolean FtpServer::doStore()
{
  if( data.connected() )
  {
    int16_t nb = data.readBytes((uint8_t*) buf, FTP_BUF_SIZE );
    if( nb > 0 )
    {
      // Serial.println( millis() << " " << nb << endl;
      file.write((uint8_t*) buf, nb );
      bytesTransfered += nb;
    }
    return true;
  }
  closeTransfer();
  return false;
}

void FtpServer::closeTransfer()
{
  uint32_t deltaT = (int32_t) ( millis() - millisBeginTrans );
  if( deltaT > 0 && bytesTransfered > 0 )
  {
    client.print( "226-File successfully transferred\r\n226 " + String(deltaT) + " ms, "+ String(bytesTransfered / deltaT) + " kbytes/s\r\n");
  }
  else
    client.print( "226 File successfully transferred\r\n");
  
  file.close();
  data.stop();
}

void FtpServer::abortTransfer()
{
  if( transferStatus > 0 )
  {
    file.close();
    data.stop(); 
    client.print( "426 Transfer aborted\r\n"  );
    #ifdef FTP_DEBUG
      Serial.println( "Transfer aborted!") ;
    #endif
  }
  transferStatus = 0;
}

// Read a char from client connected to ftp server
//
//  update cmdLine and command buffers, iCL and parameters pointers
//
//  return:
//    -2 if buffer cmdLine is full
//    -1 if line not completed
//     0 if empty line received
//    length of cmdLine (positive) if no empty line received 

int8_t FtpServer::readChar()
{
  int8_t rc = -1;

  if( client.available())
  {
    char c = client.read();
    #ifdef FTP_DEBUG
      Serial.print( c);
      //char h[3];
      //sprintf(h,"%02X", c);
      //Serial.print( "(" + String(h) + ")" );
    #endif
    if( c == '\\' )
      c = '/';
    if( c != '\r' )
      if( c != '\n' )
      {
        if( iCL < FTP_CMD_SIZE )
          cmdLine[ iCL ++ ] = c;
        else
          rc = -2; //  Line too long
      }
      else
      {
        cmdLine[ iCL ] = 0;
        command[ 0 ] = 0;
        parameters = NULL;
        // empty line?
        if( iCL == 0 )
          rc = 0;
        else
        {
          rc = iCL;
          // search for space between command and parameters
          parameters = strchr( cmdLine, ' ' );
          if( parameters != NULL )
          {
            if( parameters - cmdLine > 4 )
              rc = -2; // Syntax error
            else
            {
              strncpy( command, cmdLine, parameters - cmdLine );
              command[ parameters - cmdLine ] = 0;
              
              while( * ( ++ parameters ) == ' ' )
                ;
            }
          }
          else if( strlen( cmdLine ) > 4 )
            rc = -2; // Syntax error.
          else
            strcpy( command, cmdLine );
          iCL = 0;
        }
      }
    if( rc > 0 ) {
      for( uint8_t i = 0 ; i < strlen( command ); i ++ )
        command[ i ] = toupper( command[ i ] );
    }    
    if( rc == -2 )
    {
      iCL = 0;
      client.print( "500 Syntax error\r\n");
    }
  }
  return rc;
}

// Make complete path/name from cwdName and parameters
//
// 3 possible cases: parameters can be absolute path, relative path or only the name
//
// parameters:
//   fullName : where to store the path/name
//
// return:
//    true, if done

boolean FtpServer::makePath( char * fullName )
{
  if ( parameters == NULL )
    return makePath( fullName, cwdName );
  else
    return makePath( fullName, parameters );
}

boolean FtpServer::makePath( char * fullName, char * param = NULL )
{
  if( param == NULL )
    param = parameters;
    
  if( strcmp( param, "/" ) == 0 ) {
    strcpy( fullName, "/" );
    return true;
  }

  // If relative path, concatenate with current dir
  if( param[0] != '/' ) {
    strcpy( fullName, cwdName );
    if( fullName[ strlen( fullName ) - 1 ] != '/' )
      strncat( fullName, "/", FTP_CWD_SIZE );
    strncat( fullName, param, FTP_CWD_SIZE );
  }
  else
    strcpy( fullName, param );

  // If ends with '/', remove it
  uint16_t strl = strlen( fullName ) - 1;
  if( fullName[ strl ] == '/' && strl > 1 )
    fullName[ strl ] = 0;

  if( strlen( fullName ) < FTP_CWD_SIZE )
    return true;

  client.print( "500 Command line too long\r\n");
  return false;
}

// Calculate year, month, day, hour, minute and second
//   from first parameter sent by MDTM command (YYYYMMDDHHMMSS)
//
// parameters:
//   pyear, pmonth, pday, phour, pminute and psecond: pointer of
//     variables where to store data
//
// return:
//    0 if parameter is not YYYYMMDDHHMMSS
//    length of parameter + space

uint8_t FtpServer::getDateTime( uint16_t * pyear, uint8_t * pmonth, uint8_t * pday,
                                uint8_t * phour, uint8_t * pminute, uint8_t * psecond )
{
  char dt[ 15 ];

  // Date/time are expressed as a 14 digits long string
  //   terminated by a space and followed by name of file
  if( strlen( parameters ) < 15 || parameters[ 14 ] != ' ' )
    return 0;
  for( uint8_t i = 0; i < 14; i++ )
    if( ! isdigit( parameters[ i ]))
      return 0;

  strncpy( dt, parameters, 14 );
  dt[ 14 ] = 0;
  * psecond = atoi( dt + 12 ); 
  dt[ 12 ] = 0;
  * pminute = atoi( dt + 10 );
  dt[ 10 ] = 0;
  * phour = atoi( dt + 8 );
  dt[ 8 ] = 0;
  * pday = atoi( dt + 6 );
  dt[ 6 ] = 0 ;
  * pmonth = atoi( dt + 4 );
  dt[ 4 ] = 0 ;
  * pyear = atoi( dt );
  return 15;
}

// Create string YYYYMMDDHHMMSS from date and time
//
// parameters:
//    date, time 
//    tstr: where to store the string. Must be at least 15 characters long
//
// return:
//    pointer to tstr

char * FtpServer::makeDateTimeStr( char * tstr, uint16_t date, uint16_t time )
{
  sprintf( tstr, "%04u%02u%02u%02u%02u%02u",
           (( date & 0xFE00 ) >> 9 ) + 1980, ( date & 0x01E0 ) >> 5, date & 0x001F,
           ( time & 0xF800 ) >> 11, ( time & 0x07E0 ) >> 5, ( time & 0x001F ) << 1 );            
  return tstr;
}

bool FtpServer::haveParameter()
{
  if( parameters != NULL && strlen( parameters ) > 0 )
    return true;
  client.print("501 No file name\r\n");
  return false;  
}

bool FtpServer::makeExistsPath( char * path, char * param )
{
  if( ! makePath( path, param ))
    return false;
  if( SD_MMC.exists( path ))
    return true;
  client.print("550 " + String(path) + " not found\r\n");

  return false;
}

boolean FtpServer::formatLIST( File * file, char * buffer )
{
          String fn, fs;
          fn = file->name();
          int i = fn.lastIndexOf("/")+1;
          fn.remove(0, i);

          char ft[21];
          char fsbuf[20];
          char fnbuf[200];
          time_t ftime = file->getLastWrite();
          strftime(ft, sizeof(ft), "%m-%d-%Y  %I:%M%p", localtime(&ftime));

          fs = String(file->size());
          fs.toCharArray(fsbuf, sizeof(fsbuf));
          fn.toCharArray(fnbuf, sizeof(fnbuf));

          /*
          // insert commas in file size string
          int insertPosition = fs.length() - 3;
          while (insertPosition > 0) {
            fs = fs.substring(0,insertPosition) + "," + fs.substring(insertPosition);
            insertPosition-=3;
          }
          */
          if(file->isDirectory()) {
            if ( ! strcmp( file->name(), "/System Volume Information" ) ) // always skip this folder
              return false;
            sprintf(buffer, "%s <DIR> %s", ft, fnbuf);
            }
          else {
            sprintf(buffer, "%s %s %s", ft, fsbuf, fnbuf);
          }
          return true;
}

boolean FtpServer::formatMLST( File * file, char * buffer )
{
          String fn, fs;
          fn = file->name();
          int i = fn.lastIndexOf("/")+1;
          fn.remove(0, i);

          char ft[21];
          char fsbuf[20];
          char fnbuf[200];
          time_t ftime = file->getLastWrite();
          strftime(ft, sizeof(ft), "%Y%m%d%H%M%S", gmtime(&ftime));

          fs = String(file->size());
          fs.toCharArray(fsbuf, sizeof(fsbuf));
          fn.toCharArray(fnbuf, sizeof(fnbuf));

          if(file->isDirectory()) {
            if ( ! strcmp( file->name(), "/System Volume Information" ) ) // always skip this folder
              return false;
            sprintf(buffer, "Type=dir;Modify=%s; %s", ft, fnbuf);
            }
          else {
            sprintf(buffer, "Type=file;Size=%s;Modify=%s; %s", fsbuf, ft, fnbuf);
          }
          return true;
}
