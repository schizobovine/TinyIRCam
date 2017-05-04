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

#define GRID_SIZE (8*8)
#define GRID_HEIGHT (8)
#define GRID_WIDTH  (8)

#define TILE_HEIGHT (8)
#define TILE_WIDTH  (8)

TinyScreen display = TinyScreen(TinyScreenPlus);
GridEye cam = GridEye();

int cells[GRID_SIZE];

typedef struct {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} color_t;

typedef struct {
  float minval;
  color_t color;
} heatmap_t;

int w, h, x_zero, y_zero, i, prev;
color_t tempColor;

// Heatmap is a gradient between these color points. Uses 16 bit color, so only
// 5-6 bits per channel max and it looks like the most significant bits get
// chopped off.
const int HEATMAP_SIZE = 3;
const heatmap_t HEATMAP[HEATMAP_SIZE] = {
    {   0.0, { 0x00, 0x00, 0x3F } }
  , {  50.0, { 0x3F, 0x00, 0x00 } }
  , { 100.0, { 0x3F, 0x3F, 0x3F } }
};

// Test display
void displayTest() {
  for (w=0; w<GRID_WIDTH; w++) {
    for (h=0; h<GRID_HEIGHT; h++) {
      dummy2color(w, h, &tempColor);
      drawCell(w, h, &tempColor);
    }
  }
}

// Sets colors
void dummy2color(int _w, int _h, color_t *c) {
  c->red   = (64 / GRID_HEIGHT) * _h;
  c->green = 0;
  c->blue  = (64 / GRID_WIDTH) * _w;
}

// Convert floaing point temperature to a RGB color value
void temp2color(float temp, color_t *c) {
  for (i=0; i<HEATMAP_SIZE; i++) {
    if (temp < HEATMAP[i].minval) {
      break;
    }
  }
  i = (i - 1) % HEATMAP_SIZE;
  prev = (i - 1) % HEATMAP_SIZE;
}

// Draw a cell
void drawCell(int w, int h, color_t *color) {

    // Cell # -> display coords
    x_zero = TILE_WIDTH * w + 16;
    y_zero = TILE_HEIGHT * h;

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
}

void setup() {

  for (i=0; i<GRID_SIZE; i++) {
    cells[i] = 0;
  }

  // Setup serial for debugging
  SerialUSB.begin(115200);
  SerialUSB.println("HAI");

  // Init and clear display
  display.begin();
  display.setBrightness(10);
  display.clearScreen();
  displayTest();

  // Init i2c
  Wire.begin();
  Wire.setClock(400000);

  // Fire up IR sensor
  int tt = cam.thermistorTemp();
  SerialUSB.print(F("thermistor temp = "));
  SerialUSB.println(tt);

  //if (!cam.begin()) {
  //  while (1) {
  //    SerialUSB.println(F("failed to find camera!"));
  //    delay(1000);
  //  }
  //}

}

void loop() {

  // Fetch fresh data
  cam.pixelOut(&cells[0]);

  for (i=0; i<GRID_SIZE; i++) {
    SerialUSB.print(cells[i] * 0.25, 4);
    if (i % GRID_WIDTH == GRID_WIDTH - 1) {
      SerialUSB.println();
    } else {
      SerialUSB.print(F(" "));
    }
  }
  SerialUSB.println(F("\n--------------------\n"));

  //SerialUSB.print(F("TWI_BUFFER_LENGTH = "));
  //SerialUSB.println(BUFFER_LENGTH, DEC);
  //SerialUSB.print(F("bytes read = "));
  //SerialUSB.println(bytes, DEC);

  // Fill in display with colors based on data read
  /*
  for (h=0; h<GRID_HEIGHT; h++) {
    for (w=0; w<GRID_WIDTH; w++) {
      SerialUSB.print(cam.cellRaw(w, h, false), HEX);
      SerialUSB.print(" ");
      SerialUSB.print(cam.cellRaw(w, h, true), HEX);
      SerialUSB.print(" ");

      // Convert floating point temperature to RGB color values
      //temp2color(cam.cell(w, h), &tempColor);
      //dummy2color(w, h, &tempColor);

      // Draw rectangle using hardware accellerated routines
      //drawCell(w, h, &tempColor);

      //SerialUSB.print("("); SerialUSB.print(w);
      //SerialUSB.print(","); SerialUSB.print(h);
      //SerialUSB.print(")");
      //SerialUSB.print(" r="); SerialUSB.print(tempColor.red, HEX);
      //SerialUSB.print(" g="); SerialUSB.print(tempColor.green, HEX);
      //SerialUSB.print(" b="); SerialUSB.print(tempColor.blue, HEX);
      //SerialUSB.println();

    }
    SerialUSB.println();
  }
  SerialUSB.println();
  */

  delay(1000);

}
