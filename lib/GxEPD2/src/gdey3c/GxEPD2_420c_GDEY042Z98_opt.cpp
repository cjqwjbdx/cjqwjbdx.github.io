// Display Library for SPI e-paper panels from Dalian Good Display and boards from Waveshare.
// Requires HW SPI and Adafruit_GFX. Caution: the e-paper panels require 3.3V supply AND data lines!
//
// Optimized version for SSD1683 partial update with voltage tuning
// Based on: https://blog.csdn.net/weixin_43550576/article/details/137375815
//
// Author: Optimized Driver
//
// Version: 1.0

#include "GxEPD2_420c_GDEY042Z98_opt.h"

GxEPD2_420c_GDEY042Z98_opt::GxEPD2_420c_GDEY042Z98_opt(int16_t cs, int16_t dc, int16_t rst, int16_t busy) :
  GxEPD2_EPD(cs, dc, rst, busy, HIGH, 25000000, WIDTH, HEIGHT, panel, hasColor, hasPartialUpdate, hasFastPartialUpdate)
{
  _use_fast_update = useFastFullUpdate;
  _partial_update_mode = false;
}

void GxEPD2_420c_GDEY042Z98_opt::setGateVoltage(uint8_t vgh)
{
  _writeCommand(0x03);
  _writeData(vgh);
}

void GxEPD2_420c_GDEY042Z98_opt::setSourceVoltage(uint8_t vsh1, uint8_t vsh2, uint8_t vsl)
{
  _writeCommand(0x04);
  _writeData(vsh1);
  _writeData(vsh2);
  _writeData(vsl);
}

void GxEPD2_420c_GDEY042Z98_opt::setVCOM(uint8_t vcom)
{
  _writeCommand(0x2C);
  _writeData(vcom);
}

void GxEPD2_420c_GDEY042Z98_opt::_setPartialVoltage()
{
  setGateVoltage(_vgh_part);
  setSourceVoltage(_vsh1_part, _vsh2_part, _vsl_part);
  setVCOM(_vcom_part);
}

void GxEPD2_420c_GDEY042Z98_opt::_setFastFullVoltage()
{
  setGateVoltage(_vgh_default);
  setSourceVoltage(_vsh1_default, _vsh2_default, _vsl_default);
  setVCOM(_vcom_default);
}

void GxEPD2_420c_GDEY042Z98_opt::initPartialUpdate()
{
  if (_hibernating) _reset();
  
  // Initialize display for partial update mode
  _writeCommand(0x12); // SWRESET
  delay(10);
  
  _writeCommand(0x01); // Driver output control
  _writeData((HEIGHT - 1) % 256);
  _writeData((HEIGHT - 1) / 256);
  _writeData(0x00);
  
  _writeCommand(0x11); // Data entry mode
  _writeData(0x01);    // X-mode: x+ y-
  
  _writeCommand(0x44); // Set RAM X address
  _writeData(0x00);
  _writeData(0x31);    // 400/8 - 1 = 49 = 0x31
  
  _writeCommand(0x45); // Set RAM Y address
  _writeData((HEIGHT - 1) % 256);
  _writeData((HEIGHT - 1) / 256);
  _writeData(0x00);
  _writeData(0x00);
  
  _writeCommand(0x3C); // Border waveform
  _writeData(0x05);
  
  _writeCommand(0x18); // Temperature sensor
  _writeData(0x80);    // Internal sensor
  
  // Set partial update mode
  _writeCommand(0x21); // Display update control
  _writeData(0x00);
  _writeData(0x00);
  
  // Set optimized voltage for partial update
  _setPartialVoltage();
  
  _partial_update_mode = true;
  _init_display_done = true;
}

void GxEPD2_420c_GDEY042Z98_opt::initFastFullUpdate()
{
  if (_hibernating) _reset();
  
  // Standard initialization
  _InitDisplay();
  
  // Set optimized voltage for fast full update
  _setFastFullVoltage();
  
  _partial_update_mode = false;
}

void GxEPD2_420c_GDEY042Z98_opt::clearScreen(uint8_t value)
{
  clearScreen(value, 0x00);
}

void GxEPD2_420c_GDEY042Z98_opt::clearScreen(uint8_t black_value, uint8_t color_value)
{
  writeScreenBuffer(black_value, color_value);
  refresh(false);
}

void GxEPD2_420c_GDEY042Z98_opt::writeScreenBuffer(uint8_t value)
{
  writeScreenBuffer(value, 0x00);
}

void GxEPD2_420c_GDEY042Z98_opt::writeScreenBuffer(uint8_t black_value, uint8_t color_value)
{
  if (!_init_display_done) _InitDisplay();
  _writeScreenBuffer(0x24, black_value);
  _writeScreenBuffer(0x26, color_value);
  _initial_write = false;
}

void GxEPD2_420c_GDEY042Z98_opt::_writeScreenBuffer(uint8_t command, uint8_t value)
{
  _setPartialRamArea(0, 0, WIDTH, HEIGHT);
  _writeCommand(command);
  _startTransfer();
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 8; i++) {
    _transfer(value);
  }
  _endTransfer();
}

void GxEPD2_420c_GDEY042Z98_opt::writeImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  _writeScreenBuffer(0x26, 0xFF);
  _writeImage(0x24, bitmap, x, y, w, h, invert, mirror_y, pgm);
}

void GxEPD2_420c_GDEY042Z98_opt::_writeImage(uint8_t command, const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (_initial_write) writeScreenBuffer();
  delay(1);
  uint16_t wb = (w + 7) / 8;
  x -= x % 8;
  w = wb * 8;
  int16_t x1 = x < 0 ? 0 : x;
  int16_t y1 = y < 0 ? 0 : y;
  int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x;
  int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y;
  int16_t dx = x1 - x;
  int16_t dy = y1 - y;
  w1 -= dx;
  h1 -= dy;
  if ((w1 <= 0) || (h1 <= 0)) return;
  if (!_init_display_done) _InitDisplay();
  _setPartialRamArea(x1, y1, w1, h1);
  _writeCommand(command);
  _startTransfer();
  for (int16_t i = 0; i < h1; i++) {
    for (int16_t j = 0; j < w1 / 8; j++) {
      uint8_t data;
      uint32_t idx = mirror_y ? j + dx / 8 + uint32_t((h - 1 - (i + dy))) * wb : j + dx / 8 + uint32_t(i + dy) * wb;
      if (pgm) {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&bitmap[idx]);
#else
        data = bitmap[idx];
#endif
      } else {
        data = bitmap[idx];
      }
      if (invert) data = ~data;
      _transfer(data);
    }
  }
  _endTransfer();
}

void GxEPD2_420c_GDEY042Z98_opt::writeImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                                                int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  _writeImagePart(0x24, bitmap, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
}

void GxEPD2_420c_GDEY042Z98_opt::_writeImagePart(uint8_t command, const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                                                 int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (_initial_write) writeScreenBuffer();
  delay(1);
  if ((w_bitmap < 0) || (h_bitmap < 0) || (w < 0) || (h < 0)) return;
  if ((x_part < 0) || (y_part < 0)) return;
  int16_t wb_bitmap = (w_bitmap + 7) / 8;
  x_part -= x_part % 8;
  w_bitmap = wb_bitmap * 8;
  int16_t x1 = x < 0 ? 0 : x;
  int16_t y1 = y < 0 ? 0 : y;
  int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x;
  int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y;
  int16_t dx = x1 - x;
  int16_t dy = y1 - y;
  w1 -= dx;
  h1 -= dy;
  if ((w1 <= 0) || (h1 <= 0)) return;
  if (!_init_display_done) _InitDisplay();
  _setPartialRamArea(x1, y1, w1, h1);
  _writeCommand(command);
  _startTransfer();
  for (int16_t i = 0; i < h1; i++) {
    for (int16_t j = 0; j < w1 / 8; j++) {
      uint8_t data;
      int16_t x2 = j + dx / 8 + x_part / 8;
      int16_t y2 = i + dy + y_part;
      if ((x2 < 0) || (x2 >= wb_bitmap) || (y2 < 0) || (y2 >= h_bitmap)) data = 0xFF;
      else {
        uint32_t idx = mirror_y ? x2 + uint32_t((h_bitmap - 1 - y2)) * wb_bitmap : x2 + uint32_t(y2) * wb_bitmap;
        if (pgm) {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
          data = pgm_read_byte(&bitmap[idx]);
#else
          data = bitmap[idx];
#endif
        } else {
          data = bitmap[idx];
        }
      }
      if (invert) data = ~data;
      _transfer(data);
    }
  }
  _endTransfer();
}

void GxEPD2_420c_GDEY042Z98_opt::writeImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  _writeImage(0x24, black, x, y, w, h, invert, mirror_y, pgm);
  _writeImage(0x26, color, x, y, w, h, invert, mirror_y, pgm);
}

void GxEPD2_420c_GDEY042Z98_opt::writeImagePart(const uint8_t* black, const uint8_t* color, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                                                int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  _writeImagePart(0x24, black, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
  _writeImagePart(0x26, color, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
}

void GxEPD2_420c_GDEY042Z98_opt::writeNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (data1) {
    writeImage(data1, x, y, w, h, invert, mirror_y, pgm);
  }
}

void GxEPD2_420c_GDEY042Z98_opt::drawImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImage(bitmap, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
}

void GxEPD2_420c_GDEY042Z98_opt::drawImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                                                int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImagePart(bitmap, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
}

void GxEPD2_420c_GDEY042Z98_opt::drawImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImage(black, color, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
}

void GxEPD2_420c_GDEY042Z98_opt::drawImagePart(const uint8_t* black, const uint8_t* color, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                                                int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImagePart(black, color, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
}

void GxEPD2_420c_GDEY042Z98_opt::drawNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeNative(data1, data2, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
}

void GxEPD2_420c_GDEY042Z98_opt::refresh(bool partial_update_mode)
{
  if (partial_update_mode || _partial_update_mode) {
    _Update_Part();
  } else {
    _Update_Full();
  }
}

void GxEPD2_420c_GDEY042Z98_opt::refresh(int16_t x, int16_t y, int16_t w, int16_t h)
{
  x -= x % 8;
  w -= w % 8;
  int16_t x1 = x < 0 ? 0 : x;
  int16_t y1 = y < 0 ? 0 : y;
  int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x;
  int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y;
  _setPartialRamArea(x1, y1, w1, h1);
  
  if (_partial_update_mode) {
    _writeCommand(0x22);
    _writeData(0xDC);  // Partial update command
    _writeCommand(0x20);
    _waitWhileBusy("refresh_part", partial_refresh_time);
  } else {
    _Update_Full();
  }
}

void GxEPD2_420c_GDEY042Z98_opt::refresh_bw(int16_t x, int16_t y, int16_t w, int16_t h)
{
  refresh(x, y, w, h);
}

void GxEPD2_420c_GDEY042Z98_opt::selectFastFullUpdate(bool ff)
{
  _use_fast_update = ff;
}

void GxEPD2_420c_GDEY042Z98_opt::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  uint16_t xe = (x + w - 1) | 0x0007;
  uint16_t ye = y + h - 1;
  uint16_t xs_bpp = x / 8;
  uint16_t xe_bpp = xe / 8;
  _writeCommand(0x44); // SET_RAM_X_ADDRESS_START_END_POSITION
  _writeData(xs_bpp);
  _writeData(xe_bpp);
  _writeCommand(0x45); // SET_RAM_Y_ADDRESS_START_END_POSITION
  _writeData(y % 256);
  _writeData(y / 256);
  _writeData(ye % 256);
  _writeData(ye / 256);
  _writeCommand(0x4E); // SET_RAM_X_ADDRESS_COUNTER
  _writeData(xs_bpp);
  _writeCommand(0x4F); // SET_RAM_Y_ADDRESS_COUNTER
  _writeData(y % 256);
  _writeData(y / 256);
}

void GxEPD2_420c_GDEY042Z98_opt::_PowerOn()
{
  if (!_power_is_on)
  {
    _writeCommand(0x22);
    _writeData(0xc0);
    _writeCommand(0x20);
    _waitWhileBusy("_PowerOn", power_on_time);
  }
  _power_is_on = true;
}

void GxEPD2_420c_GDEY042Z98_opt::_PowerOff()
{
  if (_power_is_on)
  {
    _writeCommand(0x22);
    _writeData(0xc3);
    _writeCommand(0x20);
    _waitWhileBusy("_PowerOff", power_off_time);
  }
  _power_is_on = false;
}

void GxEPD2_420c_GDEY042Z98_opt::_InitDisplay()
{
  if (_hibernating) _reset();
  _writeCommand(0x12); // SWRESET
  delay(10);
  _writeCommand(0x01); // Driver output control
  _writeData((HEIGHT - 1) % 256);
  _writeData((HEIGHT - 1) / 256);
  _writeData(0x00);
  _writeCommand(0x3C); // Border waveform
  _writeData(0x05);
  _writeCommand(0x18); // Temperature sensor
  _writeData(0x80);
  _setPartialRamArea(0, 0, WIDTH, HEIGHT);
  _init_display_done = true;
  _partial_update_mode = false;
}

void GxEPD2_420c_GDEY042Z98_opt::_Update_Full()
{
  if (_use_fast_update)
  {
    _writeCommand(0x1A); // Write to temperature register
    _writeData(0x5a);    // 90
    _writeData(0x00);
    _writeCommand(0x22); // Display Update Sequence Options
    _writeData(0x91);    // Load LUT for temperature value
    _writeCommand(0x20); // Master Activation
    delay(2);
    _writeCommand(0x22); // Display Update Sequence Options
    _writeData(0xC7);
    _writeCommand(0x20); // Master Activation
    _waitWhileBusy("_Update_Fast", full_refresh_time);
  }
  else
  {
    _writeCommand(0x22); // Display Update Sequence Options
    _writeData(0xF7);
    _writeCommand(0x20); // Master Activation
    _waitWhileBusy("_Update_Full", full_refresh_time);
  }
  _power_is_on = false;
}

void GxEPD2_420c_GDEY042Z98_opt::_Update_Part()
{
  // Optimized partial update sequence
  _writeCommand(0x22); // Display Update Sequence Options
  _writeData(0xDC);    // Partial update command
  _writeCommand(0x20); // Master Activation
  _waitWhileBusy("_Update_Part", partial_refresh_time);
  _power_is_on = false;
}

void GxEPD2_420c_GDEY042Z98_opt::powerOff()
{
  _PowerOff();
}

void GxEPD2_420c_GDEY042Z98_opt::hibernate()
{
  _PowerOff();
  if (_rst >= 0)
  {
    _writeCommand(0x10); // deep sleep mode
    _writeData(0x01);
    _hibernating = true;
    _init_display_done = false;
    _power_is_on = false;
  }
}
