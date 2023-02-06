#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Adafruit_FT6206.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

#define MOTION_NONE 0
#define MOTION_UP 1
#define MOTION_DOWN 2
#define MOTION_LEFT 3
#define MOTION_RIGHT 4

int direction = 0;
int cell_x = 16;
int cell_y = 12;

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ts = Adafruit_FT6206();

void setup() {
  Serial.begin(9600);
  Serial.println("ILI9341"); 
 
  tft.begin();

  if (!ts.begin(40)) { 
    Serial.println("Unable to start touchscreen.");
  } 
  else { 
    Serial.println("Touchscreen started."); 
  }

  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 

   tft.fillScreen(ILI9341_BLACK);
   tft.setRotation(1); 

  setCell(cell_x,cell_y);
}


void loop(void) {
  // See if there's any  touch data for us
  if (ts.touched())
  {   
    // Retrieve a point  
    TS_Point p = ts.getPoint(); 
    // rotate coordinate system
    // flip it around to match the screen.
    p.x = map(p.x, 0, 240, 240, 0);
    p.y = map(p.y, 0, 320, 320, 0);
    int y = tft.height() - p.x;
    int x = p.y;

    // x: 0-105 106-214 215-320
    // y: 0-80 81-159 160-240

    if(x<106 && y>80 && y<160) direction=MOTION_LEFT;
    else if(x>214 && y>80 && y<160) direction=MOTION_RIGHT;
    else if(x>105 && x<215 && y>159) direction=MOTION_DOWN;
    else if(x>105 && x<215 && y<81) direction=MOTION_UP;
    else direction=MOTION_NONE;

    switch (direction) {
      case MOTION_LEFT:
        Serial.print("Left");
        break;
      case MOTION_RIGHT:
        Serial.print("Right");
        break;
      case MOTION_UP:
        Serial.print("Up");
        break;
      case MOTION_DOWN:
        Serial.print("Down");
        break;
      case MOTION_NONE:
        Serial.print("None");
        break;
      default:
        break;
    }

    Serial.print("\t");
    Serial.print(x);
    Serial.print("\t");
    Serial.print(y);
    Serial.print("\n");
  
  }

  move();
  delay(50);
  
}

void setCell(int x, int y) {
  tft.fillRect(x*10, y*10, 10, 10, ILI9341_GREEN);
}

void unsetCell(int x, int y) {
  tft.fillRect(x*10, y*10, 10, 10, ILI9341_BLACK);
}

void move() {
  int x = cell_x;
  int y = cell_y;

  switch (direction) {
    case MOTION_LEFT:
      cell_x--;
      break;
    case MOTION_RIGHT:
      cell_x++;
      break;
    case MOTION_UP:
      cell_y--;
      break;
    case MOTION_DOWN:
      cell_y++;
      break;
    case MOTION_NONE:
      break;
    default:
      break;
  }

  if(cell_x<0) cell_x = 32;
  if(cell_x>32) cell_x = 0;
  if(cell_y<0) cell_y = 24;
  if(cell_y>24) cell_y = 0;

  unsetCell(x,y);
  setCell(cell_x,cell_y);

}