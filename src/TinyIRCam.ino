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

TinyScreen display = TinyScreen(TinyScreenPlus);

#define GRID_SIZE (8*8)
#define GRID_HEIGHT (8)
#define GRID_WIDTH  (8)

#define TILE_HEIGHT (8)
#define TILE_WIDTH  (8)

#define FLATTEN(w, h) (((w) % GRID_WIDTH) + ((h) * GRID_HEIGHT))

int16_t rawData[GRID_SIZE];

void rectFor(int w, int h, int value) {
  int x0 = TILE_WIDTH * w + 16;
  int y0 = TILE_HEIGHT * h;
  display.drawRect(x0, y0, TILE_WIDTH, TILE_HEIGHT, TSRectangleFilled, value);
}

void drawGrid() {
  for (int w=0; w<GRID_WIDTH; w++) {
    for (int h=0; h<GRID_HEIGHT; h++) {
      rectFor(w, h, rawData[FLATTEN(w, h)]);
    }
  }
}

void setup() {
  display.begin();
  display.setBrightness(10);
  display.clearScreen();
  for (size_t h=0; h<GRID_HEIGHT; h++) {
    for (size_t w=0; w<GRID_WIDTH; w++) {
      rawData[FLATTEN(w, h)] = w * h;
    }
  }
}

void loop() {
  drawGrid();
}
