/**************************************
 *  Example FTP Server on ESP32-CAM
 *
 *  This example has some extra code in it, but the essential steps are:
 *  - Include ESP32FtpServer.h and declare an FTPServer (Ex. FTPServer ftpSrv)
 *  - Start a WiFi connection
 *  - Mount the SD Card
 *  - Include a frequent call to the FTPServer handler in loop (Ex. thftpSrv.handleFTP(); )
 **************************************/


#include <WiFi.h>  
#include "esp_deep_sleep.h"
#include "time.h"
#include "driver/gpio.h"

#include "ESP32FtpServer.h"

FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP32FtpServer.h to see ftp verbose on serial

// MicroSD
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"

#define uS_TO_S_FACTOR 1000000LL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP5S  5         /* Time ESP32 will go to sleep (in seconds) */

const char* ssid = "yourSSID";
const char* password = "yourPWD";

// Setup static IP
// Set your Static IP address
IPAddress local_IP(192, 168, 2, 33);
// Set your Gateway IP address
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

// NTP stuff
#define CAstateTZ (-80) /* west of GMT */
const char* ntpServer = "pool.ntp.org";
int   myTZ = CAstateTZ;  
const long  gmtOffset_sec = 360 * CAstateTZ; 
const int   daylightOffset_sec = 3600;

static esp_err_t card_err;


void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.print(&timeinfo, "%A, %B %d %Y %H:%M:%S");//Friday, February 22 2019 22:37:45
  div_t TZ = div(abs(myTZ), 10);  //JvZ add TZ info (trickier than it looks e.g. +/-0.5)
  Serial.printf(" [%c%d%s]\n", myTZ<0 ? '-':'+', TZ.quot, TZ.rem ? ".5":"" );//JvZ add TZ
}

/*
  %a Abbreviated weekday name
  %A Full weekday name
  %b Abbreviated month name
  %B Full month name
  %c Date and time representation for your locale
  %d Day of month as a decimal number (01-31)
  %H Hour in 24-hour format (00-23)
  %I Hour in 12-hour format (01-12)
  %j Day of year as decimal number (001-366)
  %m Month as decimal number (01-12)
  %M Minute as decimal number (00-59)
  %p Current locale's A.M./P.M. indicator for 12-hour clock
  %S Second as decimal number (00-59)
  %U Week of year as decimal number,  Sunday as first day of week (00-51)
  %w Weekday as decimal number (0-6; Sunday is 0)
  %W Week of year as decimal number, Monday as first day of week (00-51)
  %x Date representation for current locale
  %X Time representation for current locale
  %y Year without century, as decimal number (00-99)
  %Y Year with century, as decimal number
  %z %Z Time-zone name or abbreviation, (no characters if time zone is unknown)
  %% Percent sign
  You can include text literals (such as spaces and colons) to make a neater display or for padding between adjoining columns.
  You can suppress the display of leading zeroes  by using the "#" character  (%#d, %#H, %#I, %#j, %#m, %#M, %#S, %#U, %#w, %#W, %#y, %#Y)
*/



void setup() {

  Serial.begin(115200);

  Serial.setDebugOutput(true);

  Serial.println("                                    ");
  Serial.println("-------------------------------------");
  Serial.println("ESP-CAM FTP Server Example");
  Serial.println("-------------------------------------");

  pinMode(33, OUTPUT);        // little red led on back of chip
  digitalWrite(33, LOW);      // turn on the red LED on the back of chip

  // WiFi init
  if (!init_wifi()) {
    major_fail();
  }
  
  // SD card init
  card_err = init_sdcard();
  if (card_err != ESP_OK) {
    Serial.printf("SD Card init failed with error 0x%x", card_err);
    major_fail();
  }

  // Start FTP server and assign username and password for ftp server

  ftpSrv.begin("esp", "esp");

  digitalWrite(33, HIGH);     // turn off the red LED on the back of the chip

  Serial.print("FTP Server Ready! Use IP '");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}


//
// if no sd card or wifi, then flash SOS on rear led
//
void major_fail() {

  for  (int i = 0;  i < 5; i++) {
    digitalWrite(33, LOW);   delay(150);
    digitalWrite(33, HIGH);  delay(150);
    digitalWrite(33, LOW);   delay(150);
    digitalWrite(33, HIGH);  delay(150);
    digitalWrite(33, LOW);   delay(150);
    digitalWrite(33, HIGH);  delay(150);

    delay(1000);

    digitalWrite(33, LOW);  delay(500);
    digitalWrite(33, HIGH); delay(500);
    digitalWrite(33, LOW);  delay(500);
    digitalWrite(33, HIGH); delay(500);
    digitalWrite(33, LOW);  delay(500);
    digitalWrite(33, HIGH); delay(500);

    delay(1000);

    digitalWrite(33, LOW);   delay(150);
    digitalWrite(33, HIGH);  delay(150);
    digitalWrite(33, LOW);   delay(150);
    digitalWrite(33, HIGH);  delay(150);
    digitalWrite(33, LOW);   delay(150);
    digitalWrite(33, HIGH);  delay(150);

    delay(1000);
  }

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP5S * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}


bool init_wifi() {
  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
    return false;
  }
  
  // Wi-Fi connection
  WiFi.begin(ssid, password);
  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (cnt > 20) {
      Serial.println("WiFi Failed to connect");
      return false;
    }
  }
  Serial.println("\nWiFi connected");
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);//ntp timeserversetting
  printLocalTime(); Serial.println();

  return true;
}


static esp_err_t init_sdcard()
{
  esp_err_t ret = ESP_FAIL;
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files = 10,
  };
  sdmmc_card_t *card;

  Serial.println("Mounting SD card...");
  ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

  if (ret == ESP_OK) {
    Serial.println("SD card mount successfully!");
  }  else  {
    Serial.printf("Failed to mount SD card VFAT filesystem. Error: %s", esp_err_to_name(ret));
    major_fail();
  }

  Serial.print("SD_MMC Begin: "); Serial.println(SD_MMC.begin());
}


void loop()
{

  ftpSrv.handleFTP();
  delay(10);

}
