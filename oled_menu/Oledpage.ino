#include <Wire.h> // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h"
SSD1306 display(0x3c, 21, 22);
const int BUTTON_Next = 16;
const int BUTTON_Prev = 17;
const int BUTTON_Setup = 5;
int page = 1;
void setup() {
  Wire.begin();
  Serial.begin(115200);
  pinMode(BUTTON_Next, INPUT_PULLDOWN);
  pinMode(BUTTON_Prev, INPUT_PULLDOWN);
  pinMode(BUTTON_Setup, INPUT_PULLDOWN);
  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.clear();
}

bool spage = true;
bool setup_page = false;
int setup_line = 1;
String Line[4] = {"","1 Set Temp","2 Set Rh","3 Default Reset"};
void loop(){
  int ppage = page;
  int psetup_line = setup_line;
  if (digitalRead(BUTTON_Next) == HIGH)
  { // Check if button has been pressed
    while (digitalRead(BUTTON_Next) == HIGH)
    { delay(100); }
    Serial.println("Press Next");
    if (setup_page) {
      setup_line++;
      if (setup_line>3) { setup_line = 3; }
    } else {
      page++;
      if (page>3) { page = 1; }
    }
  }
  if (digitalRead(BUTTON_Prev) == HIGH)
  { // Check if button has been pressed
    while (digitalRead(BUTTON_Prev) == HIGH)
    { delay(100); }
    Serial.println("Press Prev");
    if (setup_page) {
      setup_line = setup_line-1;
      if (setup_line==0) { setup_line = 1; }
    } else {
      page = page-1;
      if (page==0) { page = 3; }
    }
  }
  if (digitalRead(BUTTON_Setup) == HIGH)
  { // Check if button has been pressed
    while (digitalRead(BUTTON_Setup) == HIGH)
    { delay(100); }
    Serial.println("Press Setup");
    if (setup_page) {
      setup_page = false;
      spage = true;
    } else {
      setup_page = true;
    }
  }
  if (setup_page) {
    display.clear();
    display.drawString(50,0,"Setup");
    int i;
    for (i=1; i<4; i++) 
    {
      display.drawString(0,15*i,Line[i]);
    }
    if (psetup_line!=setup_line) {
      display.setColor(BLACK);
      display.fillRect(0,15*psetup_line,100,13);
      display.setColor(WHITE);   
      display.drawString(0,15*psetup_line,Line[psetup_line]);
      display.display();     
    }
    display.fillRect(0,15*setup_line,100,13);
    display.setColor(BLACK);
    display.drawString(0,15*setup_line,Line[setup_line]);
    display.setColor(WHITE);   
    display.display();
    delay(100);
  } else {
    if (ppage!=page || spage) {
      if (spage) { display.clear(); }
      delay(100);
      spage = false;
      display.setColor(BLACK);
      display.fillRect(50,0,40,13);
      display.display();
      display.setColor(WHITE);
      display.drawString(50,0,"Page "+String(page));
      display.display();
      delay(100);
    }
  }
}
