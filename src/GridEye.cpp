/*
 * GridEye.cpp
 *
 * Arduino library for the Panasonic Grid EYE infrared tempeture senssor
 * arrays, using i2c.
 *
 * References/credit:
 *
 * https://github.com/kriswiner/AMG8833
 * https://www.eewiki.net/display/Motley/Panasonic+Grid+Eye+Memory+Registers
 *
 * Author: Sean Caulfield <sean@yak.net>
 * License: GPL v2.0
 *
 */

#include <Wire.h>
#include "GridEye.h"

GridEye::GridEye() {
  for (size_t i=0; i<GRIDEYE_PIXELS; i++) {
    this->rawData[2*i] = 0x00;
    this->rawData[2*i+1] = 0x00;
    this->temps[i] = 0.0;
  }
}

/*
 * Initialize sensor.
 */
bool GridEye::begin() {

  // Ping device to make sure it's there
  Wire.begin();
  Wire.beginTransmission(this->i2c_addr);
  if (Wire.endTransmission() != 0)
    return false;

  // Send reset
  if (this->write8(GRIDEYE_RST, GRIDEYE_RST_INIT))
    return false;

  // Bring out of sleep
  if (this->write8(GRIDEYE_PCTL, GRIDEYE_MODE_DEFAULT))
    return false;

  // Set capture to 10FPS
  if (this->write8(GRIDEYE_FPSC, GRIDEYE_10FPS))
    return false;

  return true;
}

/*
 * Fetch data from sensor.
 */
size_t GridEye::poll() {

  // Set i2c register pointer to begining of data block
  Wire.beginTransmission(this->i2c_addr);
  Wire.write(GRIDEYE_PIXEL01L);
  Wire.endTransmission();

  // Fetch data from sensor
  size_t count = Wire.requestFrom(this->i2c_addr, GRIDEYE_RAW_BYTES);
  for (size_t i=0; i<GRIDEYE_RAW_BYTES; i++) {
    if (i <= count) {
      this->rawData[i] = Wire.read();
    } else {
      this->rawData[i] = 0;
    }
  }

  // Crunch some FP
  for (size_t i=0; i<GRIDEYE_PIXELS; i++) {
    uint8_t lsb = this->rawData[2*i];
    uint8_t msb = this->rawData[2*i+1];
    this->temps[i] = (float) ((((int16_t) msb) << 8) | lsb) * 0.25;
  }
  
  return count;

}

/*
 * Get a cell's temperature reading.
 */
float GridEye::cell(size_t w, size_t h) {
  if (w < GRIDEYE_WIDTH && h < GRIDEYE_HEIGHT) {
    return this->temps[FLATTEN(w, h)];
  } else {
    return 0.0;
  }
}

/*
 * Get raw value for a cell
 */
uint8_t GridEye::cellRaw(size_t w, size_t h, bool _lsb) {
  uint8_t b = 0;

  if (w < GRIDEYE_WIDTH && h < GRIDEYE_HEIGHT) {
    if (_lsb) {
      b = this->rawData[2*FLATTEN(w, h)];
    } else {
      b = this->rawData[2*FLATTEN(w, h)+1];
    }
  }

  return b;
}

uint8_t GridEye::write8(uint8_t reg, uint8_t data) {
  Wire.beginTransmission(this->i2c_addr);
  Wire.write(reg);
  Wire.write(data);
  return Wire.endTransmission();
}
