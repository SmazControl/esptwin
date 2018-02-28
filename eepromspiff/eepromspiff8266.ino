#include "FS.h"
#include "SPIFFS.h"
#include <EEPROM.h>
int maxe = 4096;
int addr = 0;
byte value;


bool readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("Failed to open file for reading");
        return(false);
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    return(true);
}

bool writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return(false);
    }
    if(file.print(message)){
        Serial.println("File written");
        return(true);
    } else {
        Serial.println("Write failed");
        return(false);
    }
}


void setup() {
  Serial.begin(115200);
  EEPROM.begin(maxe);
  if(!SPIFFS.begin()){
      Serial.println("SPIFFS Mount Failed");
      return;
  }
  // always use this to "mount" the filesystem

  // this opens the file "f.txt" in read-mode
  bool f = readFile(SPIFFS, "/f.txt");
  
  if (!f) {
    Serial.println("File doesn't exist yet. Creating it");

    // open the file in write mode
    bool f = writeFile(SPIFFS, "/f.txt", "ssid=abc\npassword=123455secret\n");
    if (!f) {
      Serial.println("file creation failed");
    }
  } else {

  }
}

void loop() {
  // nothing to do for now, this is just a simple test
  if (addr == -1) {
    
  } else {
    if (addr < maxe-1) {
      EEPROM.write(addr, 0);
      // Serial.println(addr);
      addr = addr + 1;
    } else {
      EEPROM.write(addr, 128);
      EEPROM.commit();
      value = EEPROM.read(addr);
      Serial.println("ok if number below == 128");
      Serial.println(value, DEC);
      addr = -1;
    }
  }
  //delay(100);  
}
