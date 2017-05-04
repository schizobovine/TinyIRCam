/*
 * GridEye.h
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

#ifndef _GRIDEYE_H
#define _GRIDEYE_H

#define GRIDEYE_I2C_ADDR (0x68)

#define GRIDEYE_PIXELS (64)
#define GRIDEYE_RAW_BYTES (GRIDEYE_PIXELS * sizeof(uint16_t))
#define GRIDEYE_WIDTH  (8)
#define GRIDEYE_HEIGHT (8)
#define FLATTEN(w, h) (((w) % GRIDEYE_WIDTH) + ((h) * GRIDEYE_HEIGHT))

#define GRIDEYE_PCTL  (0x00)
#define GRIDEYE_RST   (0x01)
#define GRIDEYE_FPSC  (0x02)
#define GRIDEYE_INTC  (0x03)
#define GRIDEYE_STAT  (0x04)
#define GRIDEYE_SCLR  (0x05)
#define GRIDEYE_AVE   (0x07)
#define GRIDEYE_INTHL (0x08)
#define GRIDEYE_INTHH (0x09)
#define GRIDEYE_INTLL (0x0A)
#define GRIDEYE_INTLH (0x0B)
#define GRIDEYE_IHYSL (0x0C)
#define GRIDEYE_IHYSH (0x0D)
#define GRIDEYE_TTHL  (0x0E)
#define GRIDEYE_TTHH  (0x0F)

#define GRIDEYE_INT0  (0x10)
#define GRIDEYE_INT1  (0x11)
#define GRIDEYE_INT2  (0x12)
#define GRIDEYE_INT3  (0x13)
#define GRIDEYE_INT4  (0x14)
#define GRIDEYE_INT5  (0x15)
#define GRIDEYE_INT6  (0x16)
#define GRIDEYE_INT7  (0x17)

#define GRIDEYE_PIXEL01L (0x80)
#define GRIDEYE_PIXEL01H (0x81)
// ...
#define GRIDEYE_PIXEL64L (0xFE)
#define GRIDEYE_PIXEL64H (0xFF)

#define GRIDEYE_MODE_NORMAL      (0x00)
#define GRIDEYE_MODE_SLEEP       (0x10)
#define GRIDEYE_MODE_STANDBY_60S (0x20)
#define GRIDEYE_MODE_STANDBY_10S (0x21)
#define GRIDEYE_MODE_DEFAULT     GRIDEYE_MODE_NORMAL

#define GRIDEYE_RST_FLAG (0x30)
#define GRIDEYE_RST_INIT (0x3F)

#define GRIDEYE_10FPS (0x00)
#define GRIDEYE_1FPS  (0x01)

class GridEye {

  public:
    GridEye();
    bool begin();
    size_t poll();
    float cell(size_t w, size_t h);
    uint8_t cellRaw(size_t w, size_t h, bool _lsb);

  protected:
    uint8_t write8(uint8_t reg, uint8_t data);

    uint8_t i2c_addr = GRIDEYE_I2C_ADDR;
    uint8_t rawData[GRIDEYE_RAW_BYTES];
    float temps[GRIDEYE_PIXELS];

};

#endif
