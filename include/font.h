#ifndef __FONT_H__
#define __FONT_H__

#include <stdbool.h>
#include <u8g2_fonts.h>

extern const uint8_t u8g2_font_qweather_icon_16[] U8G2_FONT_SECTION("u8g2_font_qweather_icon_16");
extern const uint8_t u8g2_font_WuJinOSForCalendar_today_date[] U8G2_FONT_SECTION("u8g2_font_WuJinOSForCalendar_today_date");
extern const uint8_t u8g2_font_WuJinOSForCalendar_special_signal[] U8G2_FONT_SECTION("u8g2_font_WuJinOSForCalendar_special_signal");

// project wide font aliases
#ifndef FONT_TEXT
#define FONT_TEXT u8g2_font_wqy16_t_gb2312
#endif
#ifndef FONT_SUB
#define FONT_SUB u8g2_font_wqy12_t_gb2312
#endif

#endif // __FONT_H__
