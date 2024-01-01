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
bool alive = true;

// snake is held in 2 arrays for coordinates. First element is the head of the snake. 
#define MAX_LENGTH 100
int x_cell_pos[MAX_LENGTH+1];
int y_cell_pos[MAX_LENGTH+1];
int length = 5;

// milliseconds between moves
#define MOVE_TIME_MILLIS 200
unsigned long lastMoveMillis;

int fruit_x, fruit_y;

void setup() {
  Serial.begin(9600);

  randomSeed(analogRead(0));
  
  setupScreen();

  setupSnake();

  dropFruit();

  lastMoveMillis = millis();
}

void setupScreen() {
  
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

}

void setupSnake() {
  for(int i=0; i<=length; i++) // the position after the length is used to clean up the prior position of the snake
  {
    x_cell_pos[i] = X_START_CELL-i;
    y_cell_pos[i] = Y_START_CELL;
  }
  drawSnake();
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
    
    int zone = 0;

    if(y < Y_THIRD) {
      if(x < X_THIRD) zone = 1;
      else if(x < X_TWO_THIRD) zone = 2;
      else zone = 3;
    }
    else if(y < Y_TWO_THIRD) {
      if(x < X_THIRD) zone = 4;
      else if(x < X_TWO_THIRD) zone = 5;
      else zone = 6;
    }
    else {
      if(x < X_THIRD) zone = 7;
      else if(x < X_TWO_THIRD) zone = 8;
      else zone = 9;
    }

    Serial.print("zone");
    Serial.println(zone);

    switch (motion) {
      case MOTION_LEFT:
        if(zone==2) motion=MOTION_UP;
        else if(zone==8) motion=MOTION_DOWN;
        break;
      case MOTION_RIGHT:
        if(zone==2) motion=MOTION_UP;
        else if(zone==8) motion=MOTION_DOWN;
        break;
      case MOTION_UP:
        if(zone==4) motion=MOTION_LEFT;
        else if(zone==6) motion=MOTION_RIGHT;
        break;
      case MOTION_DOWN:
        if(zone==4) motion=MOTION_LEFT;
        else if(zone==6) motion=MOTION_RIGHT;
        break;
      case MOTION_NONE:
        if(zone==2) motion=MOTION_UP;
        else if(zone==4) motion=MOTION_LEFT;
        else if(zone==6) motion=MOTION_RIGHT;
        else if(zone==8) motion=MOTION_DOWN;
        break;
      default:
        break;
    }  
  }

  if(alive && millis()-lastMoveMillis >= MOVE_TIME_MILLIS) {
    move();
    lastMoveMillis = millis();
  }

}

void setCell(int x, int y, unsigned int col) {
  tft.fillRect(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, col);
}

void drawSnake() {
  int c = 1;
  for(int i=0; i<length; i++) {
    if(i==0) setCell(x_cell_pos[i],y_cell_pos[i], alive ? ILI9341_DARKGREEN : ILI9341_DARKGREY);
    else if(c==1) setCell(x_cell_pos[i],y_cell_pos[i], alive ? ILI9341_GREEN : ILI9341_DARKGREY);
    else setCell(x_cell_pos[i],y_cell_pos[i],alive ? ILI9341_YELLOW : ILI9341_LIGHTGREY);
    c=-c;
  }
  setCell(x_cell_pos[length],y_cell_pos[length],ILI9341_BLACK);
}

void dropFruit() {
  while(true) {
    fruit_x = random(X_MAX_CELL);
    fruit_y = random(Y_MAX_CELL);
    if(!isWithinSnake(fruit_x, fruit_y)) {
      setCell(fruit_x,fruit_y,ILI9341_RED);
      return;
    }
  }
}

bool isWithinSnake(int x, int y) {
  for(int i = 0; i < length; i++) {
    if(x == x_cell_pos[i] && y == y_cell_pos[i]) {
      return true;
    }
  }
  return false;
}

void move() {

  if(motion == MOTION_NONE) return;

  int new_x = x_cell_pos[0];
  int new_y = y_cell_pos[0];

  switch (motion) {
    case MOTION_LEFT:
     new_x--;
      break;
    case MOTION_RIGHT:
      new_x++;
      break;
    case MOTION_UP:
      new_y--;
      break;
    case MOTION_DOWN:
      new_y++;
      break;
    default:
      break;
  }

  // check for boundary collision
  if(new_x < 0 || new_x > X_MAX_CELL-1 || new_y < 0 || new_y > Y_MAX_CELL-1)
  { // crashed
    motion = MOTION_NONE;
    alive = false;
    drawSnake();
    return;
  }

  //check for snake cross
  if(isWithinSnake(new_x, new_y)) {
      motion = MOTION_NONE;
      alive = false;
      drawSnake();
      return;
  }

  // stack push
  for(int i = length; i > 0; i--) {
    x_cell_pos[i] = x_cell_pos[i-1];
    y_cell_pos[i] = y_cell_pos[i-1];
  }

  // add new position to top
  x_cell_pos[0] = new_x;
  y_cell_pos[0] = new_y;

  // eaten fruit?
  if(new_x==fruit_x && new_y==fruit_y) {
    length++;
    dropFruit();
  }

  drawSnake();

}