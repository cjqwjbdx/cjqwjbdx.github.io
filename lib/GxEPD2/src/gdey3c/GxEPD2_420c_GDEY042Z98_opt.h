// Display Library for SPI e-paper panels from Dalian Good Display and boards from Waveshare.
// Requires HW SPI and Adafruit_GFX. Caution: the e-paper panels require 3.3V supply AND data lines!
//
// Optimized version for SSD1683 partial update with voltage tuning
// Based on: https://blog.csdn.net/weixin_43550576/article/details/137375815
//
// Author: Optimized Driver
//
// Version: 1.0

#ifndef _GxEPD2_420c_GDEY042Z98_OPT_H_
#define _GxEPD2_420c_GDEY042Z98_OPT_H_

#include "../GxEPD2_EPD.h"

class GxEPD2_420c_GDEY042Z98_opt : public GxEPD2_EPD
{
  public:
    // attributes
    static const uint16_t WIDTH = 400;
    static const uint16_t WIDTH_VISIBLE = WIDTH;
    static const uint16_t HEIGHT = 300;
    static const GxEPD2::Panel panel = GxEPD2::GDEY042Z98;
    static const bool hasColor = true;
    static const bool hasPartialUpdate = true;
    static const bool hasFastPartialUpdate = true;  // Optimized for partial update
    static const bool useFastFullUpdate = true;
    static const uint16_t power_on_time = 100;
    static const uint16_t power_off_time = 250;
    static const uint16_t full_refresh_time = 15000;  // Optimized timing
    static const uint16_t partial_refresh_time = 1200; // Optimized partial refresh
    
    // constructor
    GxEPD2_420c_GDEY042Z98_opt(int16_t cs, int16_t dc, int16_t rst, int16_t busy);
    
    // methods for voltage tuning (based on article)
    void setGateVoltage(uint8_t vgh);      // CMD 0x03
    void setSourceVoltage(uint8_t vsh1, uint8_t vsh2, uint8_t vsl);  // CMD 0x04
    void setVCOM(uint8_t vcom);            // CMD 0x2C
    void initPartialUpdate();              // Initialize for partial update
    void initFastFullUpdate();             // Initialize for fast full update
    
    // standard methods
    void clearScreen(uint8_t value = 0xFF);
    void clearScreen(uint8_t black_value, uint8_t color_value);
    void writeScreenBuffer(uint8_t value = 0xFF);
    void writeScreenBuffer(uint8_t black_value, uint8_t color_value);
    void writeImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void writeImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                        int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void writeImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void writeImagePart(const uint8_t* black, const uint8_t* color, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                        int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void writeNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void drawImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void drawImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                       int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void drawImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void drawImagePart(const uint8_t* black, const uint8_t* color, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                       int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void drawNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void refresh(bool partial_update_mode = false);
    void refresh(int16_t x, int16_t y, int16_t w, int16_t h);
    void refresh_bw(int16_t x, int16_t y, int16_t w, int16_t h);
    void powerOff();
    void hibernate();
    void selectFastFullUpdate(bool);
    
  private:
    void _writeScreenBuffer(uint8_t command, uint8_t value);
    void _writeImage(uint8_t command, const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void _writeImagePart(uint8_t command, const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
                         int16_t x, int16_t y, int16_t w, int16_t h, bool invert = false, bool mirror_y = false, bool pgm = false);
    void _setPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void _PowerOn();
    void _PowerOff();
    void _InitDisplay();
    void _Update_Full();
    void _Update_Part();
    
    // Optimized voltage settings for different refresh modes
    void _setPartialVoltage();
    void _setFastFullVoltage();
    
  private:
    bool _use_fast_update;
    bool _partial_update_mode;
    
    // Voltage settings (based on article)
    static const uint8_t _vgh_default = 0x15;      // 15V
    static const uint8_t _vsh1_default = 0x41;     // 15V
    static const uint8_t _vsh2_default = 0xA8;     // 5V
    static const uint8_t _vsl_default = 0x32;      // -15V
    static const uint8_t _vcom_default = 0x36;     // VCOM
    
    // Optimized voltage for partial update
    static const uint8_t _vgh_part = 0x17;         // Slightly higher for partial
    static const uint8_t _vsh1_part = 0x45;        // Boosted for clarity
    static const uint8_t _vsh2_part = 0xA8;
    static const uint8_t _vsl_part = 0x32;
    static const uint8_t _vcom_part = 0x38;        // Adjusted for partial
};

#endif
