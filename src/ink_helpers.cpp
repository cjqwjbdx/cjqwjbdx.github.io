// ink_helpers.cpp
// 实现 ink_helpers 中声明的绘制/打印辅助函数
// 目标：把与 u8g2Fonts / display 交互的通用小函数集中到单独的实现文件，方便维护与测试

#include "ink_helpers.h"
#include "screen_ink.h"
#include <U8g2_for_Adafruit_GFX.h>
#include <GxEPD2_3C.h>
#include "GxEPD2_display_selection_new_style.h"
#include "font.h"
#include <stdarg.h>

// display is defined in src/screen_ink.cpp; declare extern here to use it
extern GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, GxEPD2_DRIVER_CLASS::HEIGHT> display;

// ---------------------------------------------------------------------------
// 基本打印/布局辅助函数（中文注释、使用示例见头文件）
// ---------------------------------------------------------------------------

// 设置字体/颜色/方向
void ink_set(const uint8_t* font, uint16_t fg, uint16_t bg, uint8_t dir) {
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setFontDirection(dir);
    u8g2Fonts.setForegroundColor(fg);
    u8g2Fonts.setBackgroundColor(bg);
    u8g2Fonts.setFont(font);
}

// 设置光标（并可顺带设置字体/颜色）
// 在指定位置绘制字符串（选择字体/前景/背景）
void ink_print(int16_t x, int16_t y, const char* s, const uint8_t* font, uint16_t fg, uint16_t bg) {
    ink_set(font, fg, bg);
    u8g2Fonts.drawUTF8(x, y, s);
}
// printf 风格的打印（在当前 cursor 上打印）
void ink_printfc(const char* fmt, ...) {
    char buf[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    u8g2Fonts.print(buf);
}


// ---------------------------------------------------------------------------
// 高级绘制助手
// ---------------------------------------------------------------------------

// 绘制日历单元格：日期数字、农历/节日文本、tag、今日高亮、节假日图标
void ink_draw_day_cell(int16_t x, int16_t y, uint8_t dayNum, uint16_t color, const char* lunarStr, const char* tagChar, const char* holidayIcon, bool isToday) {
    // 日期数字

    // 日期数字
    u8g2Fonts.setForegroundColor(color);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
    u8g2Fonts.setFont(u8g2_font_fub17_tn);
    char dayBuf[8];
    itoa(dayNum, dayBuf, 10);
    int16_t numX = x + (56 - u8g2Fonts.getUTF8Width(dayBuf)) / 2;
    int16_t numY = y + 22;
    u8g2Fonts.drawUTF8(numX, numY, dayBuf);

    // 节气/农历文字
    if (lunarStr != NULL && lunarStr[0] != '\0') {
        u8g2Fonts.setFont(FONT_TEXT);
        u8g2Fonts.drawUTF8(x + (56 - u8g2Fonts.getUTF8Width(lunarStr)) / 2, y + 44 - 4, lunarStr);
    }

    // 今日高亮（双框）
    if (isToday) {
        display.drawRoundRect(x, y + 1, 56, 44, 4, GxEPD_RED);
        display.drawRoundRect(x + 1, y + 2, 54, 42, 3, GxEPD_RED);
    }

    // 节假日图标（优先显示）
    if (holidayIcon != NULL && holidayIcon[0] != '\0') {
        u8g2Fonts.setFont(u8g2_font_open_iconic_all_1x_t);
        u8g2Fonts.setForegroundColor(GxEPD_BLACK);
        u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
        u8g2Fonts.drawUTF8(x + 44, y + 11, holidayIcon);
    }

    // Tag 图标
    if (tagChar != NULL && tagChar[0] != '\0') {
        u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
        u8g2Fonts.setForegroundColor(GxEPD_RED);
        u8g2Fonts.setFont(u8g2_font_twelvedings_t_all);
        int iconX = numX - u8g2Fonts.getUTF8Width(tagChar) - 1; // 数字与tag间间隔1像素
        iconX = iconX <= (x + 3) ? (iconX + 1) : iconX; // 防止icon与今日框线产生干涉。
        int iconY = y + 15;
        u8g2Fonts.drawUTF8(iconX, iconY, tagChar);
    }
}

// 今日样式的倒计日显示（居中）
void ink_draw_cd_today(int16_t beginX, int16_t endX, int16_t y, const char* label) {
    const char* prefix = "今日 ";
    const char* suffix = " ！！！";
    u8g2Fonts.setFont(FONT_SUB);
    int16_t preWidth = u8g2Fonts.getUTF8Width(prefix);
    int16_t suffixWidth = u8g2Fonts.getUTF8Width(suffix);
    u8g2Fonts.setFont(FONT_SUB);
    int16_t labelWidth = u8g2Fonts.getUTF8Width(label);
    int16_t margin = (endX - beginX - preWidth - labelWidth - suffixWidth) / 2;

    u8g2Fonts.setCursor((margin > 0 ? margin : 0) + beginX, y); // 居中显示
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setFont(FONT_SUB);
    u8g2Fonts.print(prefix); // 今天
    u8g2Fonts.setForegroundColor(GxEPD_RED);
    u8g2Fonts.setFont(FONT_TEXT);
    u8g2Fonts.print(label); // ****
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setFont(FONT_SUB);
    u8g2Fonts.print(suffix); // ！
}

// 倒计日（距某日还有 N 天）
void ink_draw_cd_distance(int16_t beginX, int16_t endX, int16_t y, const char* label, int days) {
    const char* prefix = "距 ";
    const char* middle = " 还有 ";
    char daysBuf[16];
    itoa(days, daysBuf, 10);
    const char* suffix = " 天";

    u8g2Fonts.setFont(FONT_SUB);
    int16_t preWidth = u8g2Fonts.getUTF8Width(prefix);
    int16_t midWidth = u8g2Fonts.getUTF8Width(middle);
    int16_t suffixWidth = u8g2Fonts.getUTF8Width(suffix);
    u8g2Fonts.setFont(FONT_SUB);
    int16_t labelWidth = u8g2Fonts.getUTF8Width(label);
    u8g2Fonts.setFont(u8g2_font_fub14_tn);
    int16_t daysWidth = u8g2Fonts.getUTF8Width(daysBuf);
    
    // 改为顶行显示，从beginX开始
    u8g2Fonts.setCursor(beginX, y);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setFont(FONT_SUB);
    u8g2Fonts.print(prefix); // 距
    u8g2Fonts.setFont(FONT_TEXT);
    u8g2Fonts.print(label); // ****
    u8g2Fonts.setFont(FONT_SUB);
    u8g2Fonts.print(middle); // 还有
    u8g2Fonts.setForegroundColor(GxEPD_RED);
    u8g2Fonts.setFont(u8g2_font_fub14_tn);
    u8g2Fonts.print(daysBuf); // 0000
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setFont(FONT_SUB);
    u8g2Fonts.print(suffix); // 天
}

// 电池图标（根据电压选择图形字符）
void ink_draw_battery_icon(int voltage, int16_t x, int16_t y) {
    String iconStr = "";
    if (voltage >= 4100) { // 满电
        iconStr = "\ue24b";
    } else if (voltage >= 3900) { // 多电
        iconStr = "\ue249";
    } else if (voltage >= 3700) { // 中电量
        iconStr = "\ue247";
    } else if (voltage >= 3500) { // 低电量
        iconStr = "\ue245";
    } else { // 空
        iconStr = "\ue242";
    }
    u8g2Fonts.setFont(u8g2_font_siji_t_6x10);
    u8g2Fonts.drawUTF8(x, y, iconStr.c_str());
}

// study schedule header cell（今日高亮）
void ink_draw_study_header_cell(int16_t x, int16_t width, int16_t topY, int16_t headerH, const char* text, bool isToday) {
    if (isToday) {
        display.fillRect(x, topY, width, headerH, GxEPD_RED);
        u8g2Fonts.setBackgroundColor(GxEPD_RED);
        u8g2Fonts.setForegroundColor(GxEPD_WHITE);
    } else {
        u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
        u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    }
    u8g2Fonts.setFont(FONT_TEXT);
    int16_t tx = x + (width - u8g2Fonts.getUTF8Width(text)) / 2;
    if (tx < x) tx = x;
    u8g2Fonts.drawUTF8(tx, topY + headerH - 4, text);
}

// study schedule cell（根据高度选择字体并居中绘制）
void ink_draw_study_cell(int16_t x, int16_t width, int yStep, int segmentIndex, int cellHeight, const char* text, uint16_t fontColor) {
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
    u8g2Fonts.setForegroundColor(fontColor);
    int fontHeight;
    if (cellHeight < 18) {
        u8g2Fonts.setFont(FONT_SUB);
        fontHeight = 12;
    } else {
        u8g2Fonts.setFont(FONT_TEXT);
        fontHeight = 16;
    }
    int16_t top = calLayout.topH + calLayout.headerH;
    int16_t drawY = top + cellHeight * yStep - 1 - (cellHeight - 1 - fontHeight) / 2 + 2 * segmentIndex;
    int16_t tx = x + (width - u8g2Fonts.getUTF8Width(text)) / 2;
    if (tx < x) tx = x;
    u8g2Fonts.drawUTF8(tx, drawY, text);
}

// 通用绘制开始设置
void ink_begin(uint16_t fg) {
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setFontDirection(0);
    u8g2Fonts.setForegroundColor(fg);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
}
