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

#define DEBUG 0

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
  uint8_t fg;
  uint8_t bg;
  char label[3];
} heatmap_t;

// Heatmap is a gradient between these color points. Uses 16 bit color, so only
// 5-6 bits per channel max and it looks like the most significant bits get
// chopped off.
const int HEATMAP_SIZE = 8;
const heatmap_t HEATMAP[HEATMAP_SIZE] = {
  //{ temp<,  {    R,    G,    B }   fg,   bg }
    { -125*4, { 0x00, 0x00, 0x00 }, 0xFF, 0x00, "<0" }
  , {    0*4, { 0x00, 0x00, 0x3F }, 0xFF, 0xE0, " 0" }
  , {   10*4, { 0x00, 0x3F, 0x3F }, 0xFF, 0xFC, "10" }
  , {   25*4, { 0x00, 0x3F, 0x00 }, 0xFF, 0x1C, "25" }
  , {   30*4, { 0x3F, 0x3F, 0x00 }, 0xFF, 0x1F, "30" }
  , {   35*4, { 0x3F, 0x00, 0x00 }, 0xFF, 0x03, "35" }
  , {   50*4, { 0x3F, 0x00, 0x3F }, 0x00, 0xE3, "50" }
  , {   99*4, { 0x3F, 0x3F, 0x3F }, 0x00, 0xFF, "99" }
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
  //if (_w == 0 || _h == 0 || _w == GRID_WIDTH || _h == GRID_HEIGHT) {
  //  display.setCursor(x_zero, y_zero);
  //  display.print(value, 0);
  //}

}

// Test display
void displayTest() {
  int w, h;
  color_t color;

  //display.setCursor(0, 0);
  //display.fontColor(HEATMAP[0].fg, HEATMAP[0].bg);
  //display.print(0, DEC);

  //display.setCursor(0, 8);
  //display.fontColor(HEATMAP[1].fg, HEATMAP[1].bg);
  //display.print(HEATMAP[1].minval, DEC);

  for (w=0; w<GRID_WIDTH; w++) {
    for (h=0; h<GRID_HEIGHT; h++) {
      color.red   = (64 / GRID_HEIGHT) * h;
      color.green = 0;
      color.blue  = (64 / GRID_WIDTH) * w;
      drawCell(w, h, 0.0, &color);
    }
  }
}

// Draw legend
void drawLegend() {

  for (int i=0; i<HEATMAP_SIZE; i++) {
    display.setCursor(0, i * 8);
    display.fontColor(HEATMAP[i].fg, HEATMAP[i].bg);
    display.print(HEATMAP[i].label);
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
  display.setFlip(1);
  display.setFont(thinPixel7_10ptFontInfo);
  drawLegend();
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
  int minT = 125*4, maxT = -125*4;
  color_t c;

  // Fetch fresh data
  cam.pixelOut(&cells[0]);

  for (int i=0; i<GRID_SIZE; i++) {

    // Current min/max on display
    if (minT > cells[i]) minT = cells[i];
    if (maxT < cells[i]) maxT = cells[i];

    // Actual temp because floating point is hard
    t = cells[i] * 0.25;

    // Get a color for that shit
    temp2color(cells[i], &c);

    // Shart out our tile
    drawCell(PIXEL_X(i), PIXEL_Y(i), t, &c);

#if DEBUG

    // Shart some more debugging crap
    DPRINT(t, 2);
    if (i % GRID_WIDTH == GRID_WIDTH - 1) {
      DPRINTLN();
    } else {
      DPRINT(F(" "));
    }

#endif

  }

  // Show min & max

  display.setCursor(80, 0);
  display.fontColor(0xFF, 0x00);
  display.print(minT/4, DEC);

  display.setCursor(80, 54);
  display.fontColor(0x00, 0xFF);
  display.print(maxT/4, DEC);

  delay(1000);

}
