// Snake classic arcade came on Arduino UNO with Adafruit ILI9341 touchscreen TFT shield (capacitive)
// https://learn.adafruit.com/adafruit-2-8-tft-touch-shield-v2

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Adafruit_FT6206.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ts = Adafruit_FT6206();

#define MOTION_NONE  0
#define MOTION_UP    1
#define MOTION_DOWN  2
#define MOTION_LEFT  3
#define MOTION_RIGHT 4

// Touchscreen resolution is 320x240
#define X_MAX_PX 320
#define Y_MAX_PX 240

// For controlling the snake we divide the screen in to 3x3 touch zones:
//   1 2 3
//   4 5 6
//   7 8 9
// Zone 4 = go left
// Zone 6 = go right
// Zone 2 = go up
// Zone 8 = go down

// Touch pixel ranges:
#define X_THIRD 106
#define X_TWO_THIRD 213
#define Y_THIRD 80
#define Y_TWO_THIRD 160

// For display, we divide the entire screen into 10x10 px cells (32x24 grid)
#define X_MAX_CELL 32
#define Y_MAX_CELL 24
#define X_START_CELL 16
#define Y_START_CELL 12
#define CELL_SIZE 10

int motion = MOTION_NONE;
int x_cell_pos = X_START_CELL;
int y_cell_pos = Y_START_CELL;


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

  setCell(x_cell_pos,y_cell_pos);
}


void loop(void) {
  // See if there's any  touch data for us
  if (ts.touched())
  {   
    // Retrieve a point  
    TS_Point p = ts.getPoint(); 
    // rotate coordinate system
    // flip it around to match the screen.
    p.x = map(p.x, 0, Y_MAX_PX, Y_MAX_PX, 0);
    p.y = map(p.y, 0, X_MAX_PX, X_MAX_PX, 0);
    int y = tft.height() - p.x;
    int x = p.y;

    if(     x <  X_THIRD     && y >= Y_THIRD     && y <  Y_TWO_THIRD) motion = MOTION_LEFT;
    else if(x >= X_TWO_THIRD && y >= Y_THIRD     && y <  Y_TWO_THIRD) motion = MOTION_RIGHT;
    else if(x >= X_THIRD     && x <  X_TWO_THIRD && y >= Y_TWO_THIRD) motion = MOTION_DOWN;
    else if(x >= X_THIRD     && x <  X_TWO_THIRD && y <  Y_THIRD    ) motion = MOTION_UP;
    else                                                              motion = MOTION_NONE;

    switch (motion) {
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
  tft.fillRect(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, ILI9341_GREEN);
}

void unsetCell(int x, int y) {
  tft.fillRect(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, ILI9341_BLACK);
}

void move() {
  int x = x_cell_pos;
  int y = y_cell_pos;

  switch (motion) {
    case MOTION_LEFT:
      x_cell_pos--;
      break;
    case MOTION_RIGHT:
      x_cell_pos++;
      break;
    case MOTION_UP:
      y_cell_pos--;
      break;
    case MOTION_DOWN:
      y_cell_pos++;
      break;
    case MOTION_NONE:
      return;
    default:
      break;
  }

  if(x_cell_pos < 0)  x_cell_pos = X_MAX_CELL;
  if(x_cell_pos > X_MAX_CELL) x_cell_pos = 0;
  if(y_cell_pos < 0)  y_cell_pos = Y_MAX_CELL;
  if(y_cell_pos > Y_MAX_CELL) y_cell_pos = 0;

  unsetCell(x,y);
  setCell(x_cell_pos,y_cell_pos);

}