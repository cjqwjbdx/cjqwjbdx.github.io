#pragma once

// Optimized SSD1683 Partial Update Support
// 
// This header provides optimized partial update functionality for SSD1683
// based on: https://blog.csdn.net/weixin_43550576/article/details/137375815
//
// Usage:
// 1. Replace your standard display initialization with the optimized version
// 2. Call initPartialUpdate() before using partial updates
// 3. Use refresh(x, y, w, h) for partial screen updates
//
// Benefits:
// - Faster refresh times (1.2s vs 2.5s for partial updates)
// - Reduced flicker
// - Lower power consumption
// - Better display consistency across batches

// If using GDEY042Z98 (SSD1683), uncomment the following:
// #define USE_SSD1683_OPTIMIZED_DRIVER

#ifdef USE_SSD1683_OPTIMIZED_DRIVER

#include <gdey3c/GxEPD2_420c_GDEY042Z98_opt.h>

// Create optimized display instance
extern GxEPD2_420c_GDEY042Z98_opt display;

// Initialize partial update mode
inline void initPartialUpdateMode() {
    display.initPartialUpdate();
}

// Initialize fast full update mode  
inline void initFastFullUpdateMode() {
    display.initFastFullUpdate();
}

// Tune voltage for specific batch (if display looks faded)
inline void tuneDisplayVoltage(uint8_t batch_number) {
    switch(batch_number) {
        case 1:
            display.setGateVoltage(0x15);
            display.setSourceVoltage(0x41, 0xA8, 0x32);
            display.setVCOM(0x36);
            break;
        case 2:
            display.setGateVoltage(0x17);  // Higher for better contrast
            display.setSourceVoltage(0x45, 0xA8, 0x32);
            display.setVCOM(0x38);
            break;
    }
}

// Example: Update only a small region
inline void updatePartialRegion(int16_t x, int16_t y, int16_t w, int16_t h, 
                                void (*drawFunc)(int16_t, int16_t, int16_t, int16_t)) {
    display.setPartialWindow(x, y, w, h);
    display.firstPage();
    do {
        drawFunc(x, y, w, h);
    } while (display.nextPage());
}

#endif // USE_SSD1683_OPTIMIZED_DRIVER
