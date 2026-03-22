// ink_helpers.h
// ---------------------------------------------
// 常用的 ink 打印与绘制助手函数声明（API 文档）
// 目的：简化对 u8g2Fonts + display 的常用绘制操作，统一字体/颜色/居中/printf 行为。
// 使用示例：
//   ink_begin(GxEPD_BLACK);
//   ink_print(10, 20, "文本", u8g2_font_fub14_tn);
//   ink_printfc("%s %d", "Day", 6);
// ---------------------------------------------
#pragma once

#include <Arduino.h>
#include <stdint.h>

// ---------------- Basic helpers ----------------
/**
 * ink_set - 设置当前字体、前景色、背景色与文本方向
 * @param font  指向 u8g2 字体数据（nullptr 表示保持当前字体）
 * @param fg    前景色（像素颜色值）
 * @param bg    背景色（像素颜色值）
 * @param dir   字体方向（0、1、2、3）
 *
 * 说明：此函数会调用 u8g2Fonts.setFontMode / setFontDirection / setForegroundColor / setBackgroundColor / setFont。
 */
void ink_set(const uint8_t* font = nullptr, uint16_t fg = 0, uint16_t bg = 0, uint8_t dir = 0);

/**
 * ink_print - 在指定位置绘制 UTF-8 文本
 * @param x,y    目标坐标
 * @param s      要绘制的 UTF-8 字符串
 * @param font   可选字体（nullptr 表示使用当前字体）
 * @param fg,bg  前景/背景色
 */
void ink_print(int16_t x, int16_t y, const char* s, const uint8_t* font = nullptr, uint16_t fg = 0, uint16_t bg = 0);

/**
 * ink_printfc - printf 风格输出到当前 cursor（不改变光标位置的起点，只在现有 cursor 输出）
 * @param fmt  printf 风格格式字符串
 */
void ink_printfc(const char* fmt, ...);


/**
 * ink_begin - 常用开始设置（例如设置前景色与背景）
 * @param fg 初始前景色（数值，常传 GxEPD_BLACK）
 */
void ink_begin(uint16_t fg = 0);

// ---------------- Advanced helpers ----------------
/**
 * ink_draw_day_cell - 绘制日历中的一个日期单元格
 * @param x,y          单元格左上坐标
 * @param dayNum       日数字（1-31）
 * @param color        日期数字颜色
 * @param lunarStr     农历或节气文本（可为 NULL 或空串）
 * @param tagChar      小图标或标记（可为 NULL）
 * @param holidayIcon  节假日图标字符（可为 NULL）
 * @param isToday      是否为当天（true 则绘制高亮框）
 *
 * 示例： ink_draw_day_cell(x, y, 6, GxEPD_BLACK, "初六", "*", "");
 */
void ink_draw_day_cell(int16_t x, int16_t y, uint8_t dayNum, uint16_t color, const char* lunarStr, const char* tagChar, const char* holidayIcon, bool isToday);

/**
 * ink_draw_cd_today - 在区域内居中绘制“今日”样式的倒计/日信息
 */
void ink_draw_cd_today(int16_t beginX, int16_t endX, int16_t y, const char* label);

/**
 * ink_draw_cd_distance - 在区域内居中绘制“距 X 还有 N 天”的倒计样式
 */
void ink_draw_cd_distance(int16_t beginX, int16_t endX, int16_t y, const char* label, int days);

/**
 * ink_draw_battery_icon - 在 (x,y) 绘制电池图标（根据电压选择图形）
 */
void ink_draw_battery_icon(int voltage, int16_t x, int16_t y);

// study schedule helpers
/**
 * ink_draw_study_header_cell - 绘制学习安排表头单元格（支持今日高亮）
 */
void ink_draw_study_header_cell(int16_t x, int16_t width, int16_t topY, int16_t headerH, const char* text, bool isToday);

/**
 * ink_draw_study_cell - 绘制学习表格中的一个单元
 */
void ink_draw_study_cell(int16_t x, int16_t width, int yStep, int segmentIndex, int cellHeight, const char* text, uint16_t fontColor);
 
