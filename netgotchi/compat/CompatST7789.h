#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#ifndef WHITE
#define WHITE ST77XX_WHITE
#endif
#ifndef BLACK
#define BLACK ST77XX_BLACK
#endif

class CompatST7789 : public Print {
public:
  CompatST7789(int8_t cs, int8_t dc, int8_t rst = -1, int16_t w = 240, int16_t h = 240, SPIClass *spi = &SPI)
      : _cs(cs), _dc(dc), _rst(rst), _w(w), _h(h), _spi(spi), _tft(cs, dc, rst) {}

  bool begin(uint8_t = 0, uint8_t = 0) {
    _spi->begin();
    _tft.init(_w, _h);
    _tft.setRotation(_rotation);
    _tft.fillScreen(ST77XX_BLACK);
    _tft.setTextWrap(false);
    return true;
  }

  void clearDisplay() { _tft.fillScreen(ST77XX_BLACK); }
  void display() {}
  void setTextSize(uint8_t s) { _tft.setTextSize(s); }

  void setTextColor(uint16_t c) {
    if (c == 0) c = ST77XX_BLACK;
    else if (c == 1) c = ST77XX_WHITE;
    _tft.setTextColor(c);
  }
  void setTextColor(uint16_t c, uint16_t bg) {
    if (c == 0) c = ST77XX_BLACK;
    else if (c == 1) c = ST77XX_WHITE;
    if (bg == 0) bg = ST77XX_BLACK;
    else if (bg == 1) bg = ST77XX_WHITE;
    _tft.setTextColor(c, bg);
  }

  void setTextWrap(bool w) { _tft.setTextWrap(w); }
  void setCursor(int16_t x, int16_t y) { _tft.setCursor(x, y); }
  void setRotation(uint8_t r) { _rotation = r; _tft.setRotation(r); }
  int16_t width() const { return _tft.width(); }
  int16_t height() const { return _tft.height(); }

  void drawPixel(int16_t x, int16_t y, uint16_t color) { _tft.drawPixel(x, y, color); }
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { _tft.drawRect(x, y, w, h, color); }
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) { _tft.fillRect(x, y, w, h, color); }
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) { _tft.drawFastHLine(x, y, w, color); }
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) { _tft.drawFastVLine(x, y, h, color); }
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) { _tft.drawLine(x0, y0, x1, y1, color); }
  void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) { _tft.drawCircle(x0, y0, r, color); }

  void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {
    _tft.drawBitmap(x, y, bitmap, w, h, color);
  }

  virtual size_t write(uint8_t c) override { return _tft.write(c); }

  void setBacklightPin(int8_t blPin, bool activeHigh = true) {
    _bl = blPin; _blActiveHigh = activeHigh;
    if (_bl >= 0) { pinMode(_bl, OUTPUT); digitalWrite(_bl, _blActiveHigh ? HIGH : LOW); }
  }
  void backlight(bool on) {
    if (_bl >= 0) digitalWrite(_bl, (_blActiveHigh ? on : !on) ? HIGH : LOW);
  }

  Adafruit_ST7789 &tft() { return _tft; }

private:
  int8_t _cs, _dc, _rst;
  int8_t _bl = -1; bool _blActiveHigh = true;
  int16_t _w, _h;
  uint8_t _rotation = 1;
  SPIClass *_spi;
  Adafruit_ST7789 _tft;
};
