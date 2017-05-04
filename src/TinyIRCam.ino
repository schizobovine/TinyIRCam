/*
 * TinyIRCam.ino
 *
 * Simple mash up of a TinyScreen+ and a 8x8 GridEye IR sensor.
 *
 * Author: Sean Caulfield <sean@yak.net>
 * License: GPL v2.0
 *
 */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <TinyScreen.h>
#include <GridEye.h>

#define DEBUG 1

#if DEBUG
#define DPRINT(...) SerialUSB.print(__VA_ARGS__)
#define DPRINTLN(...) SerialUSB.println(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINTLN(...)
#endif

#define GRID_SIZE (8*8)
#define GRID_HEIGHT (8)
#define GRID_WIDTH  (8)

#define TILE_HEIGHT (8)
#define TILE_WIDTH  (8)

#define PIXEL_X(index) ((index) % GRID_WIDTH)
#define PIXEL_Y(index) ((index) / GRID_HEIGHT)

TinyScreen display = TinyScreen(TinyScreenPlus);
GridEye cam = GridEye();

int cells[GRID_SIZE];

typedef struct {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} color_t;

typedef struct {
  int minval;
  color_t color;
} heatmap_t;

// Heatmap is a gradient between these color points. Uses 16 bit color, so only
// 5-6 bits per channel max and it looks like the most significant bits get
// chopped off.
const int HEATMAP_SIZE = 8;
const heatmap_t HEATMAP[HEATMAP_SIZE] = {
  //{ temp<,  {    R,    G,    B } }
    { -125*4, { 0x00, 0x00, 0x00 } } // black
  , {    0*4, { 0x00, 0x00, 0x3F } } // blue
  , {   10*4, { 0x00, 0x3F, 0x3F } } // cyan
  , {   25*4, { 0x00, 0x3F, 0x00 } } // green
  , {   30*4, { 0x3F, 0x3F, 0x00 } } // yellow
  , {   35*4, { 0x3F, 0x00, 0x00 } } // red
  , {   50*4, { 0x00, 0x00, 0x3F } } // purple
  , {  125*4, { 0x3F, 0x3F, 0x3F } } // white
};

// Convert floaing point temperature to a RGB color value
void temp2color(int temp, color_t *c) {
  int i, r, minT, maxT;

  // NB special start / end parameters here, as the first and last values in
  // the heat map correspond to the values at "infinity"; the temp is still
  // used for computing color intensity between levels, and anything strictly
  // lower (or higher for the upper bound) will just get the defined color.
  for (i=1; i<(HEATMAP_SIZE-1); i++) {
    if (temp < HEATMAP[i].minval) {
      break;
    }
  }

  //
  // Since we have the first and last elements as sentinels, we can be lazy
  // with pulling from the array because we know that:
  //
  // 1) If we were less than the smallest element searched (index 1), we do
  //    math vs. the "previous" element's minval and colors -- there's still an
  //    index 0
  //
  // 2) If we were larger than ALL the elements searched (size - 1), the final
  //    value of i will be the last index of the heatmap configuration, and do
  //    math based on the penultimate record.
  //

  minT = HEATMAP[i-1].minval;
  maxT = HEATMAP[i].minval;
  r = constrain(temp, minT, maxT);
  c->red   = map(r, minT, maxT, HEATMAP[i-1].color.red,   HEATMAP[i].color.red);
  c->green = map(r, minT, maxT, HEATMAP[i-1].color.green, HEATMAP[i].color.green);
  c->blue  = map(r, minT, maxT, HEATMAP[i-1].color.blue,  HEATMAP[i].color.blue);

}

// Draw a cell
void drawCell(int _w, int _h, float value, color_t *color) {
  int x_zero, y_zero;

  // Cell # -> display coords
  x_zero = TILE_WIDTH * _w + 16;
  y_zero = TILE_HEIGHT * _h;

  // Draw rectangle using hardware accellerated routines
  display.drawRect(
    x_zero
    , y_zero
    , TILE_WIDTH
    , TILE_HEIGHT
    , TSRectangleFilled
    , color->red
    , color->green
    , color->blue
  );

  // Write some text
  if (_w == 0 || _h == 0 || _w == GRID_WIDTH || _h == GRID_HEIGHT) {
    display.setCursor(x_zero, y_zero);
    display.print(value, 0);
  }

}

// Test display
void displayTest() {
  int w, h;
  color_t color;

  for (w=0; w<GRID_WIDTH; w++) {
    for (h=0; h<GRID_HEIGHT; h++) {
      color.red   = (64 / GRID_HEIGHT) * h;
      color.green = 0;
      color.blue  = (64 / GRID_WIDTH) * w;
      drawCell(w, h, 0.0, &color);
    }
  }
}

void setup() {

  for (int i=0; i<GRID_SIZE; i++) {
    cells[i] = 0;
  }

  // Setup serial for debugging
#if DEBUG
  SerialUSB.begin(115200);
  SerialUSB.println("TinyIRCam");
#endif

  // Init and clear display
  display.begin();
  display.setBrightness(10);
  display.clearScreen();
  display.setFont(thinPixel7_10ptFontInfo);
  displayTest();

  // Init i2c
  Wire.begin();
  Wire.setClock(400000);

  // Fire up IR sensor
  int tt = cam.thermistorTemp();
  DPRINT(F("thermistor temp = "));
  DPRINTLN(tt);

}

void loop() {
  float t;
  color_t c;

  // Fetch fresh data
  cam.pixelOut(&cells[0]);

  for (int i=0; i<GRID_SIZE; i++) {
    t = cells[i] * 0.25;
    temp2color(cells[i], &c);
    drawCell(PIXEL_X(i), PIXEL_Y(i), t, &c);
    DPRINT(t, 2);
    if (i % GRID_WIDTH == GRID_WIDTH - 1) {
      DPRINTLN();
    } else {
      DPRINT(F(" "));
    }
  }

  //DPRINTLN(F("\n"));

  //for (int i=0; i<GRID_SIZE; i++) {
  //  temp2color(cells[i], &c);
  //  DPRINT(F("("));
  //  DPRINT(c.red, HEX);
  //  DPRINT(F(","));
  //  DPRINT(c.green, HEX);
  //  DPRINT(F(","));
  //  DPRINT(c.blue, HEX);
  //  DPRINT(F(")"));
  //  if (i % GRID_WIDTH == GRID_WIDTH - 1) {
  //    DPRINTLN();
  //  } else {
  //    DPRINT(F(" "));
  //  }
  //}

  DPRINTLN(F("\n--------------------\n"));

  delay(1000);

}
