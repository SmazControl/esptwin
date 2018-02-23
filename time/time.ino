#include <WiFi.h>
#include "time.h"
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */
RTC_DATA_ATTR int bootCount = 0;

WiFiUDP ntpClient;

const char* ssid       = "wisoot";
const char* password   = "parrot66";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

void chkTime(){
  //lets check the time
  const int NTP_PACKET_SIZE = 48;
  byte ntpPacketBuffer[NTP_PACKET_SIZE];

  IPAddress address;
  WiFi.hostByName(ntpServer, address);
  memset(ntpPacketBuffer, 0, NTP_PACKET_SIZE);
  ntpPacketBuffer[0] = 0b11100011;   // LI, Version, Mode
  ntpPacketBuffer[1] = 0;     // Stratum, or type of clock
  ntpPacketBuffer[2] = 6;     // Polling Interval
  ntpPacketBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  ntpPacketBuffer[12]  = 49;
  ntpPacketBuffer[13]  = 0x4E;
  ntpPacketBuffer[14]  = 49;
  ntpPacketBuffer[15]  = 52;
  ntpClient.beginPacket(address, 123); //NTP requests are to port 123
  ntpClient.write(ntpPacketBuffer, NTP_PACKET_SIZE);
  ntpClient.endPacket();

  delay(1000);
  
  int packetLength = ntpClient.parsePacket();
  if (packetLength){
    if(packetLength >= NTP_PACKET_SIZE){
      ntpClient.read(ntpPacketBuffer, NTP_PACKET_SIZE);
    }
    ntpClient.flush();
    uint32_t secsSince1900 = (uint32_t)ntpPacketBuffer[40] << 24 | (uint32_t)ntpPacketBuffer[41] << 16 | (uint32_t)ntpPacketBuffer[42] << 8 | ntpPacketBuffer[43];
    time_t now;
    time(&now);
    Serial.print("Boot number #");
    Serial.println(bootCount);
    Serial.println(secsSince1900 - 2208988800UL);
    Serial.println(now);
    Serial.print("Diff.");
    Serial.println((now + 2208988800UL) - secsSince1900);
    //Serial.printf("Seconds since Jan 1 1900: %u\n", secsSince1900);
    uint32_t epoch = secsSince1900 - 2208988800UL;
    //Serial.printf("EPOCH: %u\n", epoch);
    uint8_t h = (epoch  % 86400L) / 3600;
    uint8_t m = (epoch  % 3600) / 60;
    uint8_t s = (epoch % 60);
    Serial.printf("UTC: %02u:%02u:%02u (GMT)\n", h, m, s);
  }
  printLocalTime();
  delay(9000);
}

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case 1  : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case 2  : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case 3  : Serial.println("Wakeup caused by timer"); break;
    case 4  : Serial.println("Wakeup caused by touchpad"); break;
    case 5  : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.println("Wakeup was not caused by deep sleep"); break;
  }
}


void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void setup()
{
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();
  
  Serial.begin(115200);

  if (bootCount==1) {
    //connect to WiFi
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");
    //init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    //disconnect WiFi as it's no longer needed
    printLocalTime();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  } else
  {
    //connect to WiFi
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");
    chkTime();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);    
  }
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");
}

void loop()
{
  delay(1000);
  // printLocalTime();
  Serial.println("Going to sleep now");
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}
