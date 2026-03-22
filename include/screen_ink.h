// simple screen related APIs
#pragma once

#include <Arduino.h>
int si_calendar_status();
void si_calendar();

int si_wifi_status();
void si_wifi();

int si_weather_status();
void si_weather();

int si_screen_status();
void si_screen();

void print_status();

void si_warning(const char* str);

// 时间显示已禁用
// void draw_time_partial(bool partial);
// void update_time_display();
// void refresh_time_info();

// layout for calendar sections (shared globally)
struct CalLayout {
	int16_t topX;
	int16_t topY;
	int16_t topW;
	int16_t topH;

	int16_t tX;
	int16_t tY;
	int16_t tW;
	int16_t tH;

	int16_t yearX;
	int16_t yearY;
	int16_t NyearY;
	int16_t weekX;
	int16_t weekY;

	int16_t lunarYearX;
	int16_t lunarYearY;
	int16_t lunarDayX;
	int16_t lunarDayY;
	int16_t cdDayX;
	int16_t cdDayY;

	int16_t statusX;
	int16_t statusY;
	int16_t statusW;
	int16_t statusH;

	int16_t weatherX;
	int16_t weatherY;
	int16_t weatherW;
	int16_t weatherH;
	int16_t weatherIconXOffset;
	int16_t weatherIconYOffset;
	int16_t weatherTextYOffset;
	int16_t weatherTempXOffset;
	int16_t weatherTempYOffset;

	int16_t headerX;
	int16_t headerY;
	int16_t headerW;
	int16_t headerH;

	int16_t daysX;
	int16_t daysY;
	int16_t daysW;
	int16_t daysH;
	int16_t dayW;
	int16_t dayH;
};

extern CalLayout calLayout;

// u8g2 and display instances are defined in src/screen_ink.cpp
// forward-declare them here for use by helpers
class U8G2_FOR_ADAFRUIT_GFX; // forward-declare to avoid pulling header here
extern U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

// Use the display typedef from GxEPD2 display selection header in source files
// (we avoid a direct extern template here to keep header small)

// ink helpers: declarations moved to a dedicated header
#include "ink_helpers.h"