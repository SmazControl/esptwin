// espTWIN
// Two-way WiFi Network 
// Hardware: NodeMCU 1.0
// D2 -- OLED SCL
// D3 -- OLED SDA
// SD3 -- JUMPER MASTER (AP)
// SD3 -- JUMPER SLAVE (STA) -- GND 
// D7 -- RX
// D8 -- TX
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include "SSD1306.h"
String newssid;
String newpassword;
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);
HTTPClient http;
SSD1306 display(0x3c, 0, 4);  // SDA(D3), SCL(D2) Standard I2C 
SoftwareSerial Twin_S(13, 15);   // RX(D7), TX(D8)
String wifiSSID = "wisoot";
String wifipassword = "parrot66";
String _getLink = "/index.html";

int EEPROM_write(int index, String text) 
{
  for (int i = index; i < text.length() + index; ++i) 
  {
    EEPROM.write(i, text[i - index]);  
  }
  EEPROM.write(index + text.length(), 0);
  EEPROM.commit();
  return text.length() + 1;
}

String EEPROM_read(int index, int length) 
{
  String text = "";
  char ch = 1;
  for (int i = index; (i < (index + length)) && ch; ++i)
  {
    if (ch = EEPROM.read(i))
    {
      text.concat(ch);
    }
  }
  return text;
}

String ER="      ";
String state="   ";

void setup() 
{
  newssid = "ESP000"+String(ESP.getChipId());
  newpassword = "000"+String(ESP.getChipId());
  Serial.begin(115200);                  
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setColor(WHITE);
  Serial_OLED("espTWIN Start..");
  pinMode(10,INPUT);
  if (digitalRead(10)==LOW) {
    state = "STA";
    Serial_OLED(state);
    WiFi.mode(WIFI_STA);
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
      Serial.println("no networks found");
    } else {
      Serial.print(n);
      Serial.println(" networks found");
      for (int i = 0; i < n; ++i) {
        if (i == 0) // First one
        {
          //wifiSSID = String(WiFi.SSID(i));
        }
        // Print SSID and RSSI for each network found
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i));
        Serial.print(")");
        //Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
        delay(10);
      }
    }
    Serial.println("");
    WiFi.begin(wifiSSID.c_str() , wifipassword.c_str());
    Serial_OLED(wifiSSID);
    Serial.println("Connect to:");
    Serial.println(wifiSSID);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    http.begin("http://www.esptwin.com/index.html");
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("[HTTP] GET ... code: %d\n", httpCode);
      if(httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    }
  } else {
    state = "AP";
    Serial_OLED(state);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(newssid.c_str(), newpassword.c_str());
    dnsServer.setTTL(300);
    dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
    dnsServer.start(DNS_PORT, "www.esptwin.com", apIP);
    webServer.onNotFound([]() {
      String message = "Hello World!\n\n";
      message += "URI: ";
      message += webServer.uri();

      webServer.send(200, "text/plain", message);
    });
    webServer.begin();
  }
  //EEPROM.begin(512);
  Twin_S.begin(9600);
  delay(100);
  Serial.println("");
  Serial.println("espTWIN");  
  Twin_S.println("espTWIN");
  //EEPROM_write(0,"espTWIN");
  //EEPROM_write(7,ER);
}

int input = 0;
String data="";
char inputchar;
int line = 0;
String text1 = "";
String text2 = "";
String text3 = "";

void loop()  //Can't put any delay in this loop() 
{
  if (state == "AP") {
    dnsServer.processNextRequest();
    webServer.handleClient();
  }
  Twin_Check();
}

void Serial_OLED(String Text)
{
  if (line==45) 
  {
    display.clear();
    display.drawString(0, 0, text1); 
    display.drawString(0, 15, text2); 
    display.drawString(0, 30, text3); 
    display.drawString(0, 45, Text);
    text1 = text2;
    text2 = text3;
    text3 = Text;
  } else
  {
    display.drawString(0, line, Text);
    line = line + 15;
  }
  display.display();
}

void Twin_Check()
{
  inputchar = Twin_S.read();
  input = int(inputchar);
  if((input!=255)&&(input!=10)&&(input!=0)&&(input!=13)) {
    // Serial_OLED(inputchar + " " +String(input));
    Serial.print(inputchar);
    Serial.print(" ");
    Serial.println(String(input));
    data += inputchar;
  }
  
  if (input==10) {
    Serial.println(data);
    data = "";
  }
}
