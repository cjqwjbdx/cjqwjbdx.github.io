#include "screen_ink.h"


#include <weather.h>
#include <API.hpp>
#include "holiday.h"
#include "nongli.h"
#include "gold.h"

#include "battery.h"
int voltage;

#include <_preference.h>

#include <U8g2_for_Adafruit_GFX.h>
#include <GxEPD2_3C.h>
#include "GxEPD2_display_selection_new_style.h"

#include "font.h"

#define ROTATION 0

// 启用或注释此宏以打开农历调试输出
//#define DEBUG_LUNAR

// 临时测试：强制在屏幕上显示黄金价格（调试用），发布后请删除或注释此行
// #define TEST_FORCE_GOLD

#include <stdarg.h>

#include "wiring.h"
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, GxEPD2_DRIVER_CLASS::HEIGHT> display(GxEPD2_DRIVER_CLASS(/*CS=D8*/ SPI_CS, /*DC=D3*/ SPI_DC, /*RST=D4*/ SPI_RST, /*BUSY=D2*/ SPI_BUSY));
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
 

// FONT_TEXT / FONT_SUB 已移动到 include/font.h
// calLayout instance is declared in header and defined here
// (see include/screen_ink.h)
CalLayout calLayout;

const String week_str[] = { "日", "一", "二", "三", "四", "五", "六" };
// const String tg_str[] = { "甲", "乙", "丙", "丁", "戊", "己", "庚", "辛", "壬", "癸" };            // 天干
// const String dz_str[] = { "子", "丑", "寅", "卯", "辰", "巳", "午", "未", "申", "酉", "戌", "亥" }; // 地支
// const String sx_str[] = { "鼠", "牛", "虎", "兔", "龙", "蛇", "马", "羊", "猴", "鸡", "狗", "猪" }; // 生肖
const String nl10_str[] = { "初", "十", "廿", "卅" }; // 农历十位
const String nl_str[] = { "十", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十" }; // 农历个位
const String nl_mon_str[] = { "", "正", "二", "三", "四", "五", "六", "七", "八", "九", "十", "冬", "腊" }; // 农历首位

// ink helpers implementations moved to src/ink_helpers.cpp

int _screen_status = -1;
int _calendar_status = -1;
String _cd_day_label;
String _cd_day_date;
String _tag_days_str;
int _week_1st;
int lunarDates[31];
int jqAccDate[24]; // 节气积累日
const int jrLength = 11;
const int jrDate[] = { 101, 214, 308, 312, 501, 504, 601, 701, 801, 910, 1001, 1224, 1225 };
const String jrText[] = { "元旦", "情人节", "妇女节", "植树节", "劳动节", "青年节", "儿童节", "建党节", "建军节", "教师节", "国庆节", "平安夜", "圣诞节" };

int _si_type = 0;
String _study_schedule;

extern String _gold_price;
extern bool _show_gold;

// Time display for partial update
static char _last_displayed_time[10] = "--:--";
static bool _time_initialized = false;


struct tm tmInfo = { 0 }; // 日历显示用的时间

// calLayout is declared earlier

TaskHandle_t SCREEN_HANDLER;

void init_cal_layout_size() {
    calLayout.topX = 0;
    calLayout.topY = 0;
    calLayout.topW = 180;
    calLayout.topH = 60;

    calLayout.yearX = 0;
    calLayout.yearY = calLayout.topH - 28;
    calLayout.NyearY = calLayout.topH - 18;

    calLayout.weekX = 0;
    calLayout.weekY = 12;

    calLayout.cdDayX = 0;
    calLayout.cdDayY = calLayout.topH - 5;

    calLayout.tX = calLayout.topX + calLayout.topW;
    calLayout.tY = calLayout.topY;
    calLayout.tW = 60;
    calLayout.tH = calLayout.topH / 2;

    calLayout.statusX = 300;
    calLayout.statusY = 0;
    calLayout.statusW = display.width() - calLayout.weatherX;
    calLayout.statusH = 14;

    calLayout.weatherX = 300;
    calLayout.weatherY = calLayout.topY;
    calLayout.weatherW = display.width() - calLayout.weatherX;
    calLayout.weatherH = calLayout.topH;
    
    // 天气区域内部布局（避免硬编码）
    calLayout.weatherIconXOffset = 0;  // 图标距离天气区域左边缘的偏移
    calLayout.weatherIconYOffset = 44;  // 图标距离天气区域顶部的偏移
    calLayout.weatherTextYOffset = -4;   // 天气文本距离天气区域底部的偏移
    calLayout.weatherTempXOffset = 38;   // 温湿度距离天气区域左边缘的偏移
    calLayout.weatherTempYOffset = 40;   // 温湿度距离天气区域顶部的偏移

    calLayout.headerX = calLayout.topX;
    calLayout.headerY = calLayout.topY + calLayout.topH;
    calLayout.headerW = display.width();
    calLayout.headerH = 20;

    calLayout.daysX = calLayout.topX;
    calLayout.daysY = calLayout.headerY + calLayout.headerH;
    calLayout.daysW = calLayout.headerW;
    calLayout.daysH = display.height() - calLayout.daysX;
    calLayout.dayW = 56;
    calLayout.dayH = 44;

    calLayout.lunarYearX = 0;
    // lunarDayX 在 draw_cal_year 中动态计算
    calLayout.lunarDayY = calLayout.weatherY + calLayout.weatherH + calLayout.weatherTextYOffset;
    calLayout.lunarYearY = calLayout.weatherY + calLayout.weatherH + calLayout.weatherTextYOffset;

}

void draw_cal_header() {
    ink_set(FONT_TEXT, GxEPD_WHITE, 0, 0);
    int16_t daysMagin = 4;
    
    for (int i = 0; i < 7; i++) {
        uint16_t color = ((i + _week_1st) % 7 == 0 || (i + _week_1st) % 7 == 6) ? GxEPD_RED : GxEPD_BLACK;
        int16_t x = (display.width() - 7 * calLayout.dayW) / 2 + i * calLayout.dayW;
        
        // header background
        if (i == 0) {
            display.fillRect(0, calLayout.headerY, x, calLayout.headerH, color);
        } else if (i == 6) {
            int width = display.width() - (x + calLayout.dayW);
            display.fillRect(x + calLayout.dayW, calLayout.headerY, width, calLayout.headerH, color);
        }
        display.fillRect(x, calLayout.headerY, calLayout.dayW, calLayout.headerH, color);

        // header text
        ink_print(calLayout.headerX + daysMagin + (calLayout.dayW - u8g2Fonts.getUTF8Width(week_str[i].c_str())) / 2 + i * calLayout.dayW, 
                 calLayout.headerY + calLayout.headerH - 3, week_str[(i + _week_1st) % 7].c_str(), FONT_TEXT, GxEPD_WHITE, color);
    }
}

uint16_t todayColor = GxEPD_BLACK;
String todayLunarYear;
String todayLunarDay;


// 规范化天气文本：把一些接口返回的长词或不希望显示的词替换为短显示词
static String normalize_weather_text(const String& in) {
    String s = in;
    String del[] = { "上", "下", "午" , "中", "大", "部" , "地" , "区", "朗", "分", "小" ,"早" , "晚" , "间"};
    for(int i = 0; i < 14; i++) s.replace(del[i], "");
    s.replace("局部多云", "晴云");
    s.replace(",", "转");
    s.replace("，", "转");
    return s;
}

// 规范化风向文本：统计哪种方向多就用哪种
static String normalize_wind_text(const String& in) {
    String dir[4] = {"东", "南", "西", "北"},tmp;
    int dirCount[4] = {0, 0, 0, 0};
    for(int i = 0; i < 4; i++){
        tmp = in;
        tmp.replace(dir[i], "");
        dirCount[i] = tmp.length();
    }

    int minCount = 0x7fffffff, flag = 0;
    for(int i = 0; i < 4; i++){
        if(dirCount[i] < minCount){
            minCount = dirCount[i];
            flag = i;
        }
    }
    if(in.length() > 3) return dir[flag] + "风";
    else return in;
}

// 更新年份
void draw_cal_year(bool partial) {
    // 确保设置背景色为白色
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setFontDirection(0);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
    
    // 日期
    u8g2Fonts.setForegroundColor(todayColor);
    u8g2Fonts.setCursor(calLayout.yearX, calLayout.NyearY);
    //u8g2Fonts.setFont(u8g2_font_fub25_tn);
    u8g2Fonts.setFont(u8g2_font_WuJinOSForCalendar_today_date);
    ink_printfc("%s", String((tmInfo.tm_year + 1900)%100).c_str());
    
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setFont(FONT_TEXT);
    u8g2Fonts.print("年");
    
    u8g2Fonts.setForegroundColor(todayColor);
    //u8g2Fonts.setFont(u8g2_font_fub25_tn);
    u8g2Fonts.setFont(u8g2_font_WuJinOSForCalendar_today_date);
    ink_printfc("%s", String(tmInfo.tm_mon + 1).c_str());
    
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setFont(FONT_TEXT);
    u8g2Fonts.print("月");
    
    u8g2Fonts.setForegroundColor(todayColor);
    //u8g2Fonts.setFont(u8g2_font_fub25_tn);
    u8g2Fonts.setFont(u8g2_font_WuJinOSForCalendar_today_date);
    ink_printfc("%s", String(tmInfo.tm_mday).c_str());
    
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setFont(FONT_TEXT);
    u8g2Fonts.print("日");

    // 计算公历年月日的中间 X 坐标
    int yearMonthDayEndX = u8g2Fonts.getCursorX();
    int yearMonthDayMiddleX = (calLayout.yearX + yearMonthDayEndX) / 2;

    // 在公历年月日右侧与天气区域左侧的空白区绘制未来三天预报（表格形式）
    DailyForecast* df = weather_data_daily();
    Serial.printf("[DEBUG] weather_data_daily() returned: %p, length: %d\n", df, df != NULL ? df->length : -1);
    if (df != NULL && df->length > 0) {
        Serial.printf("[DEBUG] Drawing 3-day forecast, data length: %d\n", df->length);

        int xStart = yearMonthDayEndX - 4;
        int xEnd = calLayout.weatherX - 4;
        int areaW = xEnd - xStart;

        if (areaW > 60) { // 确保有足够空间绘制表格
            u8g2Fonts.setFont(FONT_SUB);
            u8g2Fonts.setForegroundColor(GxEPD_BLACK);

            // 表格参数
            int dayCount = (df->length > 3) ? 3 : df->length;
            // 列间距变量：3表示间距为总宽度的1/(3+1)=1/4，越大间距越小
            int daySpacingDivisor = 4;
            int colW = (areaW / (dayCount + daySpacingDivisor)) * dayCount;
            int lineY1 = 18;  // 第一横线位置（日期行下方）
            int chartTop = 32; // 折线图顶部
            int chartBottom = 58; // 折线图底部（更扁）
            int chartH = chartBottom - chartTop;

            // 准备日期标签（今天、明天、后天）
            const char* dayLabels[] = {"今天", "明天", "后天"};

            // 第一行：绘制日期标签（带具体日期）
            for (int di = 0; di < dayCount; di++) {
                DailyWeather& dw = df->weather[di];
                // date格式为 "2025-02-05"，解析年月日
                String dateStr = dw.date;
                int year = dateStr.substring(0, 4).toInt();
                int month = dateStr.substring(5, 7).toInt();
                int day = dateStr.substring(8, 10).toInt();

                char dateBuf[20];
                snprintf(dateBuf, sizeof(dateBuf), "%s(%d号)", dayLabels[di], day);
                
                int centerX = xStart + di * colW + colW / 2;
                int labelW = u8g2Fonts.getUTF8Width(dateBuf);
                u8g2Fonts.setCursor(centerX - labelW / 2, lineY1 - 2);
                u8g2Fonts.print(dateBuf);
            }

            // 第二行：绘制天气描述和温度（合并一行）
            for (int di = 0; di < dayCount; di++) {
                DailyWeather& dw = df->weather[di];
                char weatherBuf[32];
                String norm = normalize_weather_text(dw.textDay);
                snprintf(weatherBuf, sizeof(weatherBuf), "%s %d-%d°C", norm.c_str(), dw.tempMin, dw.tempMax);

                int centerX = xStart + di * colW + colW / 2;
                int txtW = u8g2Fonts.getUTF8Width(weatherBuf);
                u8g2Fonts.setCursor(centerX - txtW / 2, chartTop - 2);
                u8g2Fonts.print(weatherBuf);
            }

            // 绘制折线图（使用温度偏高温：30%低温 + 70%高温）
            int points[3];
            int minTemp = 100, maxTemp = -100;

            // 计算偏高温并找出温度范围
            for (int di = 0; di < dayCount; di++) {
                DailyWeather& dw = df->weather[di];
                points[di] = dw.tempMin * 0.3 + dw.tempMax * 0.7; // 偏高温
                if (points[di] < minTemp) minTemp = points[di];
                if (points[di] > maxTemp) maxTemp = points[di];
            }

            // 绘制折线
            if (dayCount > 1) {
                // 调整温度范围，留出上下边距
                int tempRange = maxTemp - minTemp;
                if (tempRange == 0) tempRange = 1; // 避免除零

                for (int di = 0; di < dayCount - 1; di++) {
                    int x1 = xStart + di * colW + colW / 2;
                    int y1 = chartBottom - ((points[di] - minTemp) * (chartH - 6) / tempRange) - 3;
                    int x2 = xStart + (di + 1) * colW + colW / 2;
                    int y2 = chartBottom - ((points[di + 1] - minTemp) * (chartH - 6) / tempRange) - 3;

                    // 限制y坐标在图表范围内
                    if (y1 < chartTop) y1 = chartTop;
                    if (y1 > chartBottom) y1 = chartBottom;
                    if (y2 < chartTop) y2 = chartTop;
                    if (y2 > chartBottom) y2 = chartBottom;

                    display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);

                    // 绘制数据点
                    display.fillCircle(x1, y1, 2, GxEPD_BLACK);
                }
                // 绘制最后一个数据点
                int lastX = xStart + (dayCount - 1) * colW + colW / 2;
                int lastY = chartBottom - ((points[dayCount - 1] - minTemp) * (chartH - 6) / tempRange) - 3;
                if (lastY < chartTop) lastY = chartTop;
                if (lastY > chartBottom) lastY = chartBottom;
                display.fillCircle(lastX, lastY, 2, GxEPD_BLACK);
            }
        }
    } else {
        Serial.println("[DEBUG] No daily forecast data available for 3-day display");
    }

    // 今日农历年份（固定位置 X=10）
    u8g2Fonts.setFont(FONT_SUB);
    u8g2Fonts.setCursor(calLayout.lunarYearX, calLayout.lunarYearY);
    u8g2Fonts.drawUTF8(u8g2Fonts.getCursorX(), u8g2Fonts.getCursorY(), todayLunarYear.c_str());

    // 系统版本号
    #define SYSTEM_VERSION "1.1.3"

    // 第几周
    u8g2Fonts.setForegroundColor(todayColor);
    u8g2Fonts.setCursor(calLayout.weekX, calLayout.weekY);
    char week_num[8];
    strftime(week_num, sizeof(week_num), "%V", &tmInfo);
    ink_printfc("第%s周", week_num);
    
    // 在第几周右边显示选择的信息
    u8g2Fonts.setFont(FONT_SUB);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    
    // 计算"第X周"的宽度，在其右侧显示
    char weekStr[16];
    snprintf(weekStr, sizeof(weekStr), "第%s周", week_num);
    int weekWidth = u8g2Fonts.getUTF8Width(weekStr);
    int infoX = calLayout.weekX + weekWidth + 8;  // 右边留8像素间距
    int infoY = calLayout.weekY;
    
    int8_t infoType = week_info_type();
    String displayStr = "";

    infoType = 0;//测试用 不能删

    if (infoType == 0) {
        // 0: 定位位置
        String locationStr = weather_location_city();

        if (!locationStr.isEmpty()) {
            // 去掉"中国"前缀
            locationStr.replace("中国", "");

            // 处理直辖市（北京、上海、天津、重庆）
            const char* municipalities[] = {"北京", "上海", "天津", "重庆"};
            bool isMunicipality = false;
            String cityName = "";
            String districtName = "";

            for (int i = 0; i < 4; i++) {
                if (locationStr.indexOf(municipalities[i]) >= 0) {
                    isMunicipality = true;
                    cityName = municipalities[i];
                    int cityPos = locationStr.indexOf(municipalities[i]);
                    int cityEnd = cityPos + strlen(municipalities[i]);
                    if (cityEnd < locationStr.length() && locationStr.substring(cityEnd, cityEnd + 1) == "市") {
                        cityEnd++;
                    }
                    districtName = locationStr.substring(cityEnd);
                    break;
                }
            }

            if (isMunicipality && !districtName.isEmpty()) {
                // 直辖市：北京市·朝阳区
                locationStr = cityName + "市·" + districtName;
            } else {
                // 普通省市：去掉省，保留市
                int provinceEnd = locationStr.indexOf("省");
                if (provinceEnd != -1) {
                    locationStr = locationStr.substring(provinceEnd + 1);
                }
                // 移除"市辖区"
                locationStr.replace("市辖区", "");
                // 将第一个"市"后面的内容用·分隔
                int cityEnd = locationStr.indexOf("市");
                if (cityEnd != -1) {
                    locationStr = locationStr.substring(0, cityEnd + 1) + "·" + locationStr.substring(cityEnd + 1);
                }
            }
            locationStr.trim();
            // 限制长度为10个中文字符（约30字节UTF-8）
            if (locationStr.length() > 30) {
                locationStr = locationStr.substring(0, 30);
            }

            // 分离字符串，标记需要特殊处理"·"号
            displayStr = locationStr;
        }

    } else if (infoType == 1) {
        // 1: 数据更新时间
        String apiTime = weather_update_time();
        if (!apiTime.isEmpty()) {
            displayStr = "数据更新时间:" + apiTime;
        } else {
            // 如果API时间不可用，使用系统时间
            char refreshTime[32];
            snprintf(refreshTime, sizeof(refreshTime), "数据在%02d:%02d更新", tmInfo.tm_hour, tmInfo.tm_min);
            displayStr = String(refreshTime);
        }
    } else {
        // 2: 系统版本
        displayStr = "v" + String(SYSTEM_VERSION);
    }

    // 绘制显示信息（位置/时间/版本）
    if (!displayStr.isEmpty()) {
        int dotPos = displayStr.indexOf("·");
        if (dotPos != -1 && infoType == 0) {
            // 位置信息：单独处理"·"号
            u8g2Fonts.setFont(FONT_SUB);

            // 绘制"·"前的部分
            String beforeDot = displayStr.substring(0, dotPos);
            u8g2Fonts.drawUTF8(infoX, infoY, beforeDot.c_str());
            int x = infoX + u8g2Fonts.getUTF8Width(beforeDot.c_str()); // 计算新的X坐标

            // 绘制"·"号（使用特殊字体）
            u8g2Fonts.setFont(u8g2_font_WuJinOSForCalendar_special_signal);
            u8g2Fonts.drawUTF8(x, infoY, "·");
            x += u8g2Fonts.getUTF8Width("·"); // 计算新的X坐标

            // 绘制"·"后的部分
            u8g2Fonts.setFont(FONT_SUB);
            u8g2Fonts.drawUTF8(x, infoY, displayStr.substring(dotPos + 1).c_str());
        } else {
            // 时间或版本信息：正常显示
            u8g2Fonts.setFont(FONT_SUB);
            ink_printfc("%s", displayStr.c_str());
        }
    }

    // 今日农历日期（最后一个字对齐到公历年月日末尾）
    u8g2Fonts.setFont(FONT_SUB);
    const char* lunarText = todayLunarDay.isEmpty() ? ("星期" + week_str[tmInfo.tm_wday]).c_str() : todayLunarDay.c_str();
    int lunarDayWidth = u8g2Fonts.getUTF8Width(lunarText);
    // 农历日期的 X 坐标 = 公历年月日末尾 - 整个农历日期宽度
    u8g2Fonts.setCursor(yearMonthDayEndX - lunarDayWidth-4, calLayout.lunarDayY);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.drawUTF8(u8g2Fonts.getCursorX(), u8g2Fonts.getCursorY(), lunarText);

    // 黄金价格显示
    if (gold_is_enabled() && !gold_get_price().isEmpty()) {
        u8g2Fonts.setCursor(calLayout.weekX, calLayout.lunarDayY);
        ink_printfc("%s", gold_get_price().c_str());
    }
}

void draw_cal_days(bool partial) {
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setFontDirection(0);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);

    size_t totalDays = 30; // 小月
    int monthNum = tmInfo.tm_mon + 1;
    if (monthNum == 1 || monthNum == 3 || monthNum == 5 || monthNum == 7 || monthNum == 8 || monthNum == 10 || monthNum == 12) { // 大月
        totalDays = 31;
    }
    if (monthNum == 2) {
        if ((tmInfo.tm_year + 1900) % 4 == 0
            && (tmInfo.tm_year + 1900) % 100 != 0
            || (tmInfo.tm_year + 1900) % 400 == 0) {
            totalDays = 29; // 闰二月
        } else {
            totalDays = 28; // 二月
        }
    }

    // 计算本月第一天星期几
    int wday1 = (36 - tmInfo.tm_mday + tmInfo.tm_wday) % 7;
    // 计算本月第一天是全年的第几天（0～365）
    int yday1 = tmInfo.tm_yday - tmInfo.tm_mday + 1;

    // 确认哪些日期需要打tag
    char tags[31] = { 0 };
    int indexBegin = 0;
    while (_tag_days_str.length() >= (indexBegin + 9)) {
        String y = _tag_days_str.substring(indexBegin, indexBegin + 4);
        String m = _tag_days_str.substring(indexBegin + 4, indexBegin + 6);
        String d = _tag_days_str.substring(indexBegin + 6, indexBegin + 8);
        char t = _tag_days_str.charAt(indexBegin + 8);

        if ((y.equals(String(tmInfo.tm_year + 1900)) || y.equals("0000")) && (m.equals(String(tmInfo.tm_mon + 1)) || m.equals("00"))) {
            tags[d.toInt()] = t;
        }

        // Serial.printf("Format: %s, %s, %s, %c\n", y.c_str(), m.c_str(), d.c_str(), t);

        indexBegin = indexBegin + 9;
        while (indexBegin < _tag_days_str.length() && (_tag_days_str.charAt(indexBegin) < '0' || _tag_days_str.charAt(indexBegin) > '9')) { // 搜索字符串直到下个字符是0-9之间的
            indexBegin++;
        }
    }

    Holiday _holiday;
    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    size_t holiday_size = pref.getBytesLength(PREF_HOLIDAY);
    if (holiday_size > 0) {
        pref.getBytes(PREF_HOLIDAY, &_holiday, holiday_size);
    }
    pref.end();

    if (_holiday.year != tmInfo.tm_year + 1900 || _holiday.month != tmInfo.tm_mon + 1) {
        _holiday = {};
    }

    int jqIndex = 0;
    int jrIndex = 0;
    int shiftDay = (wday1 - _week_1st) >= 0 ? 0 : 7;
    for (size_t iDay = 0; iDay < totalDays; iDay++) {
        uint8_t num = wday1 + iDay - _week_1st + shiftDay; // 根据每周首日星期做偏移
        uint8_t column = num % 7; //(0~6)
        uint8_t row = num / 7;    //(0~4)
        if (row == 5) row = 0;
        int16_t x = calLayout.daysX + 4 + column * 56;
        int16_t y = calLayout.daysY + row * 44;

    #ifdef DEBUG_LUNAR
        /* placeholder removed: actual per-cell debug prints occur after lunarStr is computed */
    #endif

        // 周六、日，字体红色
        uint16_t color;
        if ((wday1 + iDay) % 7 == 0 || (wday1 + iDay) % 7 == 6) {
            color = GxEPD_RED;
        } else {
            color = GxEPD_BLACK;
        }

        const char* holidayIcon = NULL;
        if (tmInfo.tm_year + 1900 == _holiday.year && tmInfo.tm_mon + 1 == _holiday.month) {
            uint8_t holidayIndex = 0;
            for (; holidayIndex < _holiday.length; holidayIndex++) {
                if (abs(_holiday.holidays[holidayIndex]) == (iDay + 1)) {
                    // 标记公休/调班，用于后续绘制
                    if (_holiday.holidays[holidayIndex] > 0) { // 公休
                        color = GxEPD_RED;
                        holidayIcon = "\u006c";
                    } else if (_holiday.holidays[holidayIndex] < 0) { // 调班
                        color = GxEPD_BLACK;
                        holidayIcon = "\u0064";
                    }
                    break;
                }
            }
        }
        u8g2Fonts.setForegroundColor(color); // 设置整体颜色
        u8g2Fonts.setBackgroundColor(GxEPD_WHITE);

        // 绘制单元格：包括日期、农历/节气、tag、节假日图标、今日高亮（如果是今日）

        // 画节气&节日&农历
        String lunarStr = "";
        int lunarDate = lunarDates[iDay];
        int isLeapMon = lunarDate < 0 ? 1 : 0; // 闰月
        lunarDate = abs(lunarDate);
        int lunarMon = lunarDate / 100;
        int lunarDay = lunarDate % 100;

        // 调试输出：打印每个单元格计算到的农历信息，便于定位错位问题
    #ifdef DEBUG_LUNAR
        Serial.printf("CAL DAY %2d -> lunarDate=%d mon=%d day=%d\n", iDay+1, lunarDate, lunarMon, lunarDay);
    #endif

        bool isJq = false; // 是否节气
        int accDays0 = tmInfo.tm_yday + 1 - tmInfo.tm_mday; // 本月0日的积累日（tm_yday 从0开始，tm_mday从1开始, i从0开始）
        for (; jqIndex < 24; jqIndex++) {
            if (accDays0 + iDay + 1 < jqAccDate[jqIndex]) {
                break;
            }
            if (accDays0 + iDay + 1 == jqAccDate[jqIndex]) {
                lunarStr = String(nl_jq_text[jqIndex]);
                isJq = true;
                break;
            }
        }
        bool isJr = false; // 是否节日
        int currentDateNum = (tmInfo.tm_mon + 1) * 100 + iDay + 1;
        for (; jrIndex < jrLength; jrIndex++) {
            if (currentDateNum < jrDate[jrIndex]) {
                break;
            }
            if (currentDateNum == jrDate[jrIndex]) {
                lunarStr = jrText[jrIndex];
                isJr = true;
                break;
            }
        }
        if (!isJq && !isJr) { // 农历
            if (lunarDay == 1) {
                // 初一，显示月份
                lunarStr = (isLeapMon == 0 ? "" : "闰") + nl_mon_str[lunarMon] + "月";
            } else {
                if (lunarDay == 10) {
                    lunarStr = "初十";
                } else if (lunarDay == 20) {
                    lunarStr = "二十";
                } else if (lunarDay == 30) {
                    lunarStr = "三十";
                } else {
                    // 其他日期：1~9 前缀 "初"，10~29 使用十位+个位组合
                    if (lunarDay < 10) {
                        lunarStr = String("初") + nl_str[lunarDay];
                    } else {
                        lunarStr = nl10_str[lunarDay / 10] + nl_str[lunarDay % 10];
                    }
                }
            }
            if (lunarMon == 1 && lunarDay == 1) {
                lunarStr = "春节";
            } else if (lunarMon == 1 && lunarDay == 15) {
                lunarStr = "元宵节";
            } else if (lunarMon == 5 && lunarDay == 5) {
                lunarStr = "端午节";
            } else if (lunarMon == 7 && lunarDay == 7) {
                lunarStr = "七夕节";
            } else if (lunarMon == 8 && lunarDay == 15) {
                lunarStr = "中秋节";
            } else if (lunarMon == 9 && lunarDay == 9) {
                lunarStr = "重阳节";
            }
        }
        // 如果是今日，先计算并保存农历相关信息（视觉由 ink_draw_day_cell 处理）
        if ((iDay + 1) == tmInfo.tm_mday) {
            todayColor = color;
            int tg = nl_tg(tmInfo.tm_year + 1900 - (lunarMon > (tmInfo.tm_mon + 1) ? 1 : 0));
            int dz = nl_dz(tmInfo.tm_year + 1900 - (lunarMon > (tmInfo.tm_mon + 1) ? 1 : 0));
            todayLunarYear = String(nl_tg_text[tg]) + String(nl_dz_text[dz]) + String(nl_sx_text[dz]) + "年";
            if (lunarDay == 10) {
                lunarStr = "初十";
            } else if (lunarDay == 20) {
                lunarStr = "二十";
            } else if (lunarDay == 30) {
                lunarStr = "三十";
            } else {
                    lunarStr = nl10_str[lunarDay / 10] + nl_str[lunarDay % 10];
            }
            todayLunarDay = (isLeapMon == 0 ? "" : "闰") + nl_mon_str[lunarMon] + "月" + lunarStr;
#ifdef DEBUG_LUNAR
            Serial.printf("TODAY lunarStr=%s todayLunarDay=%s\n", lunarStr.c_str(), todayLunarDay.c_str());
#endif
        }

        // 画日期Tag（映射）
        const char* tagChar = NULL;
        if (tags[iDay + 1] == 'a') { //tag
            tagChar = "\u0042";
        } else if (tags[iDay + 1] == 'b') { // dollar
            tagChar = "\u0024";
        } else if (tags[iDay + 1] == 'c') { // smile
            tagChar = "\u0053";
        } else if (tags[iDay + 1] == 'd') { // warning
            tagChar = "\u0021";
        }
        // if (tagChar != NULL) {
        //     u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
        //     u8g2Fonts.setForegroundColor(GxEPD_RED);
        //     u8g2Fonts.setFont(u8g2_font_twelvedings_t_all);
        //     int iconX = numX - u8g2Fonts.getUTF8Width(tagChar) - 1; // 数字与tag间间隔1像素
        //     iconX = iconX <= (x + 3) ? (iconX + 1) : iconX; // 防止icon与今日框线产生干涉。
        //     int iconY = y + 15;
        //     u8g2Fonts.drawUTF8(iconX, iconY, tagChar);
        // }

        // 调试输出每个单元格的计算结果（位置与将要绘制的农历文本）
    #ifdef DEBUG_LUNAR
        Serial.printf("CELL %2d -> num=%d col=%d row=%d x=%d y=%d lunarDate=%d lunar=%s\n", iDay+1, num, column, row, x, y, lunarDate, lunarStr.c_str());
    #endif
        // 最终由 ink_draw_day_cell 完成所有绘制
        ink_draw_day_cell(x, y, iDay + 1, color, lunarStr.c_str(), tagChar, holidayIcon, (iDay + 1) == tmInfo.tm_mday);

        // 画Calendar提示点
        /*
        Calendar* cal = weather_cal();
        for(int calIndex = 0; calIndex < cal->length; calIndex ++) {
            CalEvent event = cal->events[calIndex];
            if(event.dt_begin.substring(0, 4).toInt() == (tmInfo.tm_year + 1900)
            && event.dt_begin.substring(4, 6).toInt() == (tmInfo.tm_mon + 1)
            && event.dt_begin.substring(6, 8).toInt() == (iDay + 1)) {
                u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
                u8g2Fonts.setForegroundColor(GxEPD_RED);
                u8g2Fonts.setFont(u8g2_font_siji_t_6x10);
                u8g2Fonts.drawUTF8(x + 42, y + 12, "\ue015");
            }
        }
        */
    }
}

// draw countdown-day info
void draw_cd_day(String label, String date) {
    if (label == NULL || date == NULL || label.length() == 0 || date.length() != 8) {
        Serial.print("Invalid countdown-day parameters.\n");
        return;
    }

    long d = atol(date.c_str());

    struct tm today = { 0 }; // 今日0秒
    today.tm_year = tmInfo.tm_year;
    today.tm_mon = tmInfo.tm_mon;
    today.tm_mday = tmInfo.tm_mday;
    today.tm_hour = 0;
    today.tm_min = 0;
    today.tm_sec = 0;
    time_t todayT = mktime(&today);

    struct tm someday = { 0 }; // 倒计日0秒
    someday.tm_year = d / 10000 - 1900;
    someday.tm_mon = d % 10000 / 100 - 1;
    someday.tm_mday = d % 100;
    someday.tm_hour = 0;
    someday.tm_min = 0;
    someday.tm_sec = 0;
    time_t somedayT = mktime(&someday);

    /*
    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", &someday);
    Serial.printf("CD day: %s\n", buffer);
    */

    long diff = somedayT - todayT;
    if (diff < 0) return; // 倒计日已过

    int16_t beginX = calLayout.cdDayX;
    int16_t endX = calLayout.weatherX;
    int16_t y = calLayout.cdDayY;

    if (diff == 0) {
        ink_draw_cd_today(beginX, endX, y, label.c_str());
    } else if (diff > 0) {
        int iDiff = diff / (60 * 60 * 24);
        ink_draw_cd_distance(beginX, endX, y, label.c_str(), iDiff);
    }
}


void draw_special_day() {
    String str = "Special Days!!!";

    u8g2Fonts.setCursor(u8g2Fonts.getCursorX() + 12, u8g2Fonts.getCursorY());
    // 使用简化 helper 来输出左右的爱心图标，中间输出文本
    ink_print(u8g2Fonts.getCursorX(), u8g2Fonts.getCursorY(), "\u00b7", u8g2_font_open_iconic_all_2x_t, GxEPD_RED, GxEPD_WHITE); // 爱心
    ink_set(FONT_TEXT, GxEPD_BLACK, GxEPD_WHITE);
    u8g2Fonts.print(str.c_str());
    ink_print(u8g2Fonts.getCursorX(), u8g2Fonts.getCursorY(), "\u00b7", u8g2_font_open_iconic_all_2x_t, GxEPD_RED, GxEPD_WHITE); // 爱心
}

bool isNight(String time) {
    uint8_t hour = time.substring(11, 13).toInt();
    return hour < 6 || hour >= 18;
}

const char* getWeatherIcon(uint16_t id, bool fill) {
    switch (id) {
    case 100: return !fill ? "\uf101" : "\uf1ac";
    case 101: return !fill ? "\uf102" : "\uf1ad";
    case 102: return !fill ? "\uf103" : "\uf1ae";
    case 103: return !fill ? "\uf104" : "\uf1af";
    case 104: return !fill ? "\uf105" : "\uf1b0";
    case 150: return !fill ? "\uf106" : "\uf1b1";
    case 151: return !fill ? "\uf107" : "\uf1b2";
    case 152: return !fill ? "\uf108" : "\uf1b3";
    case 153: return !fill ? "\uf109" : "\uf1b4";
    case 300: return !fill ? "\uf10a" : "\uf1b5";
    case 301: return !fill ? "\uf10b" : "\uf1b6";
    case 302: return !fill ? "\uf10c" : "\uf1b7";
    case 303: return !fill ? "\uf10d" : "\uf1b8";
    case 304: return !fill ? "\uf10e" : "\uf1b9";
    case 305: return !fill ? "\uf10f" : "\uf1ba";
    case 306: return !fill ? "\uf110" : "\uf1bb";
    case 307: return !fill ? "\uf111" : "\uf1bc";
    case 308: return !fill ? "\uf112" : "\uf1bd";
    case 309: return !fill ? "\uf113" : "\uf1be";
    case 310: return !fill ? "\uf114" : "\uf1bf";
    case 311: return !fill ? "\uf115" : "\uf1c0";
    case 312: return !fill ? "\uf116" : "\uf1c1";
    case 313: return !fill ? "\uf117" : "\uf1c2";
    case 314: return !fill ? "\uf118" : "\uf1c3";
    case 315: return !fill ? "\uf119" : "\uf1c4";
    case 316: return !fill ? "\uf11a" : "\uf1c5";
    case 317: return !fill ? "\uf11b" : "\uf1c6";
    case 318: return !fill ? "\uf11c" : "\uf1c7";
    case 350: return !fill ? "\uf11d" : "\uf1c8";
    case 351: return !fill ? "\uf11e" : "\uf1c9";
    case 399: return !fill ? "\uf11f" : "\uf1ca";
    case 400: return !fill ? "\uf120" : "\uf1cb";
    case 401: return !fill ? "\uf121" : "\uf1cc";
    case 402: return !fill ? "\uf122" : "\uf1cd";
    case 403: return !fill ? "\uf123" : "\uf1ce";
    case 404: return !fill ? "\uf124" : "\uf1cf";
    case 405: return !fill ? "\uf125" : "\uf1d0";
    case 406: return !fill ? "\uf126" : "\uf1d1";
    case 407: return !fill ? "\uf127" : "\uf1d2";
    case 408: return !fill ? "\uf128" : "\uf1d3";
    case 409: return !fill ? "\uf129" : "\uf1d4";
    case 410: return !fill ? "\uf12a" : "\uf1d5";
    case 456: return !fill ? "\uf12b" : "\uf1d6";
    case 457: return !fill ? "\uf12c" : "\uf1d7";
    case 499: return !fill ? "\uf12d" : "\uf1d8";
    case 500: return !fill ? "\uf12e" : "\uf1d9";
    case 501: return !fill ? "\uf12f" : "\uf1da";
    case 502: return !fill ? "\uf130" : "\uf1db";
    case 503: return !fill ? "\uf131" : "\uf1dc";
    case 504: return !fill ? "\uf132" : "\uf1dd";
    case 507: return !fill ? "\uf133" : "\uf1de";
    case 508: return !fill ? "\uf134" : "\uf1df";
    case 509: return !fill ? "\uf135" : "\uf1e0";
    case 510: return !fill ? "\uf136" : "\uf1e1";
    case 511: return !fill ? "\uf137" : "\uf1e2";
    case 512: return !fill ? "\uf138" : "\uf1e3";
    case 513: return !fill ? "\uf139" : "\uf1e4";
    case 514: return !fill ? "\uf13a" : "\uf1e5";
    case 515: return !fill ? "\uf13b" : "\uf1e6";
    case 800: return "\uf13c";
    case 801: return "\uf13d";
    case 802: return "\uf13e";
    case 803: return "\uf13f";
    case 804: return "\uf140";
    case 805: return "\uf141";
    case 806: return "\uf142";
    case 807: return "\uf143";
    case 900: return !fill ? "\uf144" : "\uf1e7";
    case 901: return !fill ? "\uf145" : "\uf1e8";
    case 999:
    default: return !fill ? "\uf146" : "\uf1e9";
    }
}

// 辅助函数：绘制天气图标
static void draw_weather_icon(int icon, bool isNightTime, int x, int y) {
    u8g2Fonts.setFont(u8g2_font_qweather_icon_16);
    u8g2Fonts.setCursor(x+2, y+2);
    u8g2Fonts.print(getWeatherIcon(icon, isNightTime));
}

// 辅助函数：绘制天气文本信息
static void draw_weather_text(const char* text, int x, int y) {
    u8g2Fonts.setFont(FONT_SUB);
    u8g2Fonts.setCursor(x-6, y);
    u8g2Fonts.print(text);
}


// 辅助函数：绘制天气温湿度
static void draw_weather_temp_humidity(int temp, int humidity, bool isNow, int x, int y) {
    u8g2Fonts.setFont(u8g2_font_tenthinnerguys_tf);
    u8g2Fonts.setCursor(x, y);
    if (isNow) {
        ink_printfc("%d°C | %d%%", temp, humidity);
    } else {
        ink_printfc("%d%%", humidity);
    }
}

// 辅助函数：绘制风向风力
static void draw_weather_wind(const char* windDir, int windScale, int centerX, int y) {
    u8g2Fonts.setFont(FONT_SUB);
    if (windScale == 0) {
        // 居中绘制"无风"
        int width = u8g2Fonts.getUTF8Width("无风");
        u8g2Fonts.setCursor(centerX - width / 2, y);
        ink_printfc("无风");
    } else {
        // 特判：如果风向为"无持续风向"，替换为"不定风"
        const char* displayWindDir = windDir;
        if (strcmp(windDir, "无持续风向") == 0) {
            displayWindDir = "不定风";
        }

        // 计算风向风力字符串的总宽度
        char windStr[32];
        snprintf(windStr, sizeof(windStr), "%s %d 级", displayWindDir, windScale);
        int windDirWidth = u8g2Fonts.getUTF8Width(displayWindDir);
        int scaleWidth = u8g2Fonts.getUTF8Width(" ");
        u8g2Fonts.setFont(u8g2_font_tenthinnerguys_tf);
        scaleWidth += u8g2Fonts.getUTF8Width(String(windScale).c_str());
        u8g2Fonts.setFont(FONT_SUB);
        scaleWidth += u8g2Fonts.getUTF8Width(" 级");
        int totalWidth = windDirWidth + scaleWidth;

        // 从中心位置开始绘制
        int startX = centerX - totalWidth / 2;

        u8g2Fonts.setCursor(startX, y);
        u8g2Fonts.print(displayWindDir);
        u8g2Fonts.setCursor(u8g2Fonts.getCursorX() + 5, u8g2Fonts.getCursorY());
        u8g2Fonts.setFont(u8g2_font_tenthinnerguys_tf);
        ink_printfc("%d", windScale);
        u8g2Fonts.setCursor(u8g2Fonts.getCursorX() + 5, u8g2Fonts.getCursorY());
        u8g2Fonts.setFont(FONT_SUB);
        ink_printfc("级");
    }
}

// 公共天气绘制函数（抽取重复代码）
static void draw_weather_common(
    int icon, bool isNightTime,
    const char* text, int temp, int tempMin, int tempMax, int humidity,
    const char* windDir, int windScale,
    bool isNow
) {
    int tempX = calLayout.weatherX + calLayout.weatherTempXOffset;

    // 天气区域各元素之间的间隔（统一间距）
    const int elementSpacing = 14;  // icon/text、temp、wind之间的间隔

    // 计算温湿度字符串的宽度和位置
    char tempHumStr[20];
    if (isNow) {
        snprintf(tempHumStr, sizeof(tempHumStr), "%d°C | %d%%", temp, humidity);
    } else {
        snprintf(tempHumStr, sizeof(tempHumStr), "%d - %d°C", tempMin, tempMax);
    }
    u8g2Fonts.setFont(u8g2_font_tenthinnerguys_tf);
    int tempHumWidth = u8g2Fonts.getUTF8Width(tempHumStr);

    // 风速的 Y 坐标（底部，保持不变）
    int windY = calLayout.weatherY + calLayout.weatherH + calLayout.weatherTextYOffset;

    // 温湿度位置（wind上方 - 间隔）
    int tempY = windY - elementSpacing;

    // 天气图标和文本位置（temp上方 - 间隔）
    int iconY = tempY - elementSpacing;
    int iconX = tempX;

    // 天气文本右对齐，与图标同一行
    u8g2Fonts.setFont(FONT_SUB);
    int textWidth = u8g2Fonts.getUTF8Width(text);
    int textX = tempX + tempHumWidth - textWidth;
    int textY = iconY;

    // 绘制天气图标
    draw_weather_icon(icon, isNightTime, iconX, iconY);

    // 绘制天气文本
    draw_weather_text(text, textX, textY);

    // 绘制温湿度
    draw_weather_temp_humidity(temp, humidity, isNow, tempX, tempY);

    // 风速显示在温湿度下方（位置不变，温湿度到风的间隔自动等于 elementSpacing）
    u8g2Fonts.setFont(FONT_SUB);
    draw_weather_wind(windDir, windScale, tempX + tempHumWidth / 2, windY);
}

// 画天气信息
#include "API.hpp"
void draw_weather(bool partial) {
    // 确保设置背景色为白色
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setFontDirection(0);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
    
    if (weather_type() == 1) {
        // 实时天气
        Weather* wNow = weather_data_now();
        if (wNow != NULL) {
            Serial.printf("[Weather] Drawing current weather: %s, %d°C, %d%%, %s %d级\n",
                         wNow->text.c_str(), wNow->temp, wNow->humidity,
                         wNow->windDir.c_str(), wNow->windScale);
            draw_weather_common(
                wNow->now_weather_id, isNight(wNow->time),
                normalize_weather_text(wNow->text).c_str(), wNow->temp, 0, 0, wNow->humidity,
                normalize_wind_text(wNow->windDir).c_str(), wNow->windScale,
                true  // isNow
            );
        } else {
            Serial.println("[Weather] No current weather data available");
        }
    } else {
        // 每日天气
        DailyForecast* wFc = weather_data_daily();
        if (wFc == NULL || wFc->length == 0) {
            Serial.println("[Weather] No daily forecast data available");
            return;
        }
        
        DailyWeather wToday = wFc->weather[0];
        Serial.printf("[Weather] Drawing daily forecast: %s, %d-%d°C, %d%%, %s %d级\n",
                     wToday.textDay.c_str(), wToday.tempMin, wToday.tempMax,
                     wToday.humidity, wToday.windDirDay.c_str(), wToday.windScaleDay);
        draw_weather_common(
            wToday.iconDay, false,
            normalize_weather_text(wToday.textDay).c_str(), 0, wToday.tempMin, wToday.tempMax, wToday.humidity,
            normalize_wind_text(wToday.windDirDay).c_str(), wToday.windScaleDay,
            false  // isNow
        );
    }
}

// Draw err
void draw_err(bool partial) {
    ink_begin(GxEPD_RED);
    // 使用 ink_print 简化写法
    ink_print(382, 18, "\u0118", u8g2_font_open_iconic_all_2x_t, GxEPD_RED, GxEPD_WHITE);
}

void draw_status(bool partial) {
    // 确保设置背景色为白色
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setFontDirection(0);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
    u8g2Fonts.setFont(u8g2_font_siji_t_6x10);

    // 电池icon（最左侧）
    String iconStr = "";
    if(voltage >= 4100) { // 满电
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
    ink_draw_battery_icon(voltage, calLayout.statusX + 4, calLayout.statusY + 2);

    //如果启用了黄金价格小部件并且有缓存数据，绘制到状态区中间
    if (_show_gold && !_gold_price.isEmpty()) {
        u8g2Fonts.setFont(FONT_SUB);
        u8g2Fonts.setForegroundColor(GxEPD_BLACK);
        u8g2Fonts.setBackgroundColor(GxEPD_WHITE);  // 明确设置背景色为白色
        // 计算金价位置：电池图标右侧，时间左侧
        int goldX = calLayout.statusX + 24;  // 电池图标宽度约20px + 4px间距
        int goldY = calLayout.statusY + calLayout.statusH - 2;
        u8g2Fonts.setCursor(goldX, goldY);
        ink_printfc("%s", _gold_price.c_str());
    }
}

// 绘制时间显示（局刷）
void draw_time_partial(bool partial) {
    Serial.printf("[DEBUG] draw_time_partial called, partial=%d, time=%02d:%02d\n", partial, tmInfo.tm_hour, tmInfo.tm_min);
    
    // 设置字体模式和颜色
    u8g2Fonts.setFontMode(1);
    u8g2Fonts.setFontDirection(0);
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
    u8g2Fonts.setFont(FONT_SUB);
    
    // 在状态栏最右侧显示时间
    char timeStr[10];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", tmInfo.tm_hour, tmInfo.tm_min);
    
    // 计算时间显示位置：右侧对齐
    int timeWidth = u8g2Fonts.getUTF8Width(timeStr);
    int timeX = display.width() - timeWidth - 4;  // 右边距4px
    int timeY = calLayout.statusY + calLayout.statusH - 2;
    
    u8g2Fonts.setCursor(timeX, timeY);
    ink_printfc("%s", timeStr);
    
    Serial.printf("[DEBUG] Drawing time at x=%d, y=%d, text=%s\n", timeX, timeY, timeStr);
    
    // 保存当前显示的时间
    strcpy(_last_displayed_time, timeStr);
}

// 更新系统时间到 tmInfo
void refresh_time_info() {
    time_t now = 0;
    time(&now);
    localtime_r(&now, &tmInfo);
    Serial.printf("[TIME] refresh_time_info called, now=%ld, time=%02d:%02d:%02d\n", 
                  now, tmInfo.tm_hour, tmInfo.tm_min, tmInfo.tm_sec);
}

// 检查时间是否变化并执行局刷更新
void update_time_display() {
    // 先更新当前时间
    refresh_time_info();
    
    // 检查时间是否变化
    char currentTime[10];
    snprintf(currentTime, sizeof(currentTime), "%02d:%02d", tmInfo.tm_hour, tmInfo.tm_min);
    
    // 标准驱动不支持局刷，每次变化都输出日志但不刷新
    if (_time_initialized && strcmp(currentTime, _last_displayed_time) == 0) {
        return;  // 时间未变化，无需刷新
    }
    
    // 首次初始化（task_screen 中已经绘制了时间，只需标记）
    if (!_time_initialized) {
        _time_initialized = true;
        Serial.printf("[INIT] Time display initialized: %s\n", currentTime);
    } else {
        // 时间变化，但标准驱动不支持局刷，仅更新时间记录
        Serial.printf("[INFO] Time changed to %s (standard driver - no partial update)\n", currentTime);
        strcpy(_last_displayed_time, currentTime);
    }
}

// 辅助函数：绘制课程表网格线条
static void draw_schedule_grid(int morningCount, int eveningCount, int nightCount, int cellHeight, int daysCount, int segmentCount) {
    // 顶部红色分隔线
    display.drawFastHLine(0, calLayout.topH, display.width(), GxEPD_RED);
    display.drawFastHLine(0, calLayout.topH - 1, display.width(), GxEPD_RED);
    display.drawFastHLine(0, calLayout.topH + calLayout.headerH - 1, display.width(), GxEPD_RED);
    display.drawFastHLine(0, calLayout.topH + calLayout.headerH, display.width(), GxEPD_RED);
    
    // 绘制水平分割线
    int max_y = calLayout.topH + calLayout.headerH;
    for (int i = 0; i < 3; i++) {
        int count = (i == 0) ? morningCount : (i == 1) ? eveningCount : nightCount;
        if (count > 0 && i > 0) {
            max_y += 2;
            display.drawFastHLine(0, max_y, display.width(), GxEPD_BLACK);
        }
        for (int j = 0; j < count; j++) {
            max_y += cellHeight;
            display.drawFastHLine(0, max_y, display.width(), GxEPD_BLACK);
        }
    }
    
    // 绘制垂直分割线
    int cellWidth = display.width() / daysCount;
    for (int x = 1; x < daysCount; x++) {
        display.drawFastVLine(cellWidth * x, calLayout.headerH + calLayout.topH, 
                             max_y - calLayout.headerH - calLayout.topH, GxEPD_BLACK);
    }
}

// 辅助函数：解析课程表配置
static void parse_study_schedule(int& morningCount, int& eveningCount, int& nightCount, 
                                String daysClassStr[], int& daysCount) {
    int config = _study_schedule.substring(0, 3).toInt();
    morningCount = config / 100;
    eveningCount = (config % 100) / 10;
    nightCount = config % 10;
    
    int pos_begin = 4;
    daysCount = 0;
    while (true) {
        int pos_end = _study_schedule.indexOf(";", pos_begin);
        if (pos_end < 0) break;
        daysClassStr[daysCount++] = _study_schedule.substring(pos_begin, pos_end);
        pos_begin = pos_end + 1;
    }
}

void drawStudySchedule() {
    int morningClassCount, eveningClassCount, nightClassCount, daysCount;
    String daysClassStr[7];
    
    parse_study_schedule(morningClassCount, eveningClassCount, nightClassCount, daysClassStr, daysCount);
    
    int totalClasses = morningClassCount + eveningClassCount + nightClassCount;
    int segmentCount = (morningClassCount > 0) + (eveningClassCount > 0) + (nightClassCount > 0);
    
    int cellHeight = (display.height() - calLayout.topH - calLayout.headerH - segmentCount * 2) / totalClasses;
    int cellWidth = display.width() / daysCount;
    int marginLeft = (display.width() - cellWidth * daysCount) / 2;
    
    draw_schedule_grid(morningClassCount, eveningClassCount, nightClassCount, cellHeight, daysCount, segmentCount);
    
    for (int x = 0; x < daysCount; x++) {
        int pos_begin = 0;
        int yStep = 0;
        int segmentIndex = 0;
        
        do {
            int pos_end = daysClassStr[x].indexOf(",", pos_begin);
            String ss = daysClassStr[x].substring(pos_begin, pos_end);
            ss.trim();
            
            if (yStep == 0) {
                bool isToday = (week_str[tmInfo.tm_wday] == ss);
                ink_draw_study_header_cell(cellWidth * x + marginLeft, cellWidth, 
                                         calLayout.topH, calLayout.headerH, ss.c_str(), isToday);
            } else {
                if (yStep == morningClassCount + 1 || yStep == (morningClassCount + eveningClassCount + 1)) {
                    segmentIndex++;
                }
                uint16_t fontColor = (week_str[tmInfo.tm_wday] == daysClassStr[x].substring(0, daysClassStr[x].indexOf(","))) ? 
                                   GxEPD_RED : GxEPD_BLACK;
                ink_draw_study_cell(cellWidth * x + marginLeft, cellWidth, yStep, segmentIndex, 
                                  cellHeight, ss.c_str(), fontColor);
            }
            
            if (pos_end < 0) break;
            pos_begin = pos_end + 1;
            yStep++;
        } while (true);
    }
}

///////////// Calendar //////////////
/**
 * 处理日历信息
 */
void si_calendar() {
    _calendar_status = 0;

    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    int32_t _calendar_date = pref.getInt(PREF_SI_CAL_DATE);
    _cd_day_label = pref.getString(PREF_CD_DAY_LABLE);
    _cd_day_date = pref.getString(PREF_CD_DAY_DATE);
    _tag_days_str = pref.getString(PREF_TAG_DAYS);
    _week_1st = pref.getString(PREF_SI_WEEK_1ST, "0").toInt();
    _study_schedule = pref.getString(PREF_STUDY_SCHEDULE);
    _si_type = pref.getInt(PREF_SI_TYPE);
    if (_study_schedule.isEmpty()) _si_type = 0;
    if (pref.isKey(PREF_WIDGETS_GOLD)) {
        _show_gold = pref.getString(PREF_WIDGETS_GOLD, "0") == "1";
        Serial.printf("Gold widget enabled: %d\n", _show_gold);
    } else {
        _show_gold = false;
        Serial.println("Gold widget preference not found.");
    }

    pref.end();
    

    time_t now = 0;
    time(&now);
    localtime_r(&now, &tmInfo); // 时间戳转化为本地时间结构
    Serial.printf("System Time: %d-%02d-%02d %02d:%02d:%02d\n", (tmInfo.tm_year + 1900), tmInfo.tm_mon + 1, tmInfo.tm_mday, tmInfo.tm_hour, tmInfo.tm_min, tmInfo.tm_sec);

    // 如果当前时间无效
    if (tmInfo.tm_year + 1900 < 2026) {
        bool isSetOK = false;
        if (weather_status() == 1) {
            // 尝试使用api获取的时间
            String apiTime;
            Weather* weatherNow = weather_data_now();
            if (weatherNow->updateTime == NULL) {
                // TODO 处理每日天气
                DailyForecast* wFc = weather_data_daily();
                apiTime = wFc->updateTime;
            } else {
                apiTime = weatherNow->updateTime;
            }
            Serial.printf("API Time: %s\n", apiTime.c_str());
            tmInfo = { 0 }; // 重置为0
            if (strptime(apiTime.c_str(), "%Y-%m-%dT%H:%M", &tmInfo) != NULL) {  // 将时间字符串转成tm时间 e.g. 2024-11-14T17:36+08:00
                time_t set = mktime(&tmInfo);
                timeval tv;
                tv.tv_sec = set;
                isSetOK = (settimeofday(&tv, nullptr) == 0);
                Serial.println("WARN: Set system time by api time.");
            } else {
                Serial.println("ERR: Fail to format api time.");
            }
        } else {
            // 如果天气也未获取成功，那么返回
            Serial.println("ERR: invalid time & not weather info got.");
        }
        if (!isSetOK) {
            _calendar_status = 2;
            return;
        }
    }

    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tmInfo);
    Serial.printf("Calendar Show Time: %s\n", buffer);

    nl_month_days(tmInfo.tm_year + 1900, tmInfo.tm_mon + 1, lunarDates);
    nl_year_jq(tmInfo.tm_year + 1900, jqAccDate);

    // 从 Preferences 读取缓存的黄金价格（如果有），以便离线也能显示
    Preferences pref2;
    pref2.begin(PREF_NAMESPACE);
    if (pref2.isKey(PREF_WIDGETS_GOLD_PRICE)) {
        _gold_price = pref2.getString(PREF_WIDGETS_GOLD_PRICE, "");
        Serial.printf("Gold cached: %s\n", _gold_price.c_str());
    }
    pref2.end();

// #ifdef TEST_FORCE_GOLD
//     // 强制测试黄金价格显示（仅用于调试）
//     _gold_price = "测试:1234.5";
//     _show_gold = true;
//     Serial.printf("FORCE GOLD enabled: %s\n", _gold_price.c_str());
// #endif

    _calendar_status = 1;
    return;
}

int si_calendar_status() {
    return _calendar_status;
}

///////////// Screen //////////////
/**
 * 屏幕刷新
 */
void task_screen(void* param) {
    Serial.println("[Task] screen update begin...");

    voltage = readBatteryVoltage();

    delay(100);

    display.init(115200);          // 串口使能 初始化完全刷新使能 复位时间 ret上拉使能
    display.setRotation(ROTATION); // 设置屏幕旋转1和3是横向  0和2是纵向
    u8g2Fonts.begin(display);

    // 在初始化后明确设置默认背景色为白色
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);

    init_cal_layout_size();
    display.setFullWindow();
    display.firstPage();
    display.fillScreen(GxEPD_WHITE);
    do {
        // 在每次循环开始时确保背景色为白色
        u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
        if (_si_type == 1) {
            drawStudySchedule();
        } else {
            draw_cal_days(false);
            draw_cal_header();
        }

        draw_cal_year(false);

        // 倒计日
        draw_cd_day(_cd_day_label, _cd_day_date);

        if (weather_status() == 1) {
            draw_weather(false);
        }
        if (voltage > 1000 && voltage < 4300) {
            draw_status(false);
        }
        
        // 时间显示已禁用
        // draw_time_partial(false);
    } while (display.nextPage());

    int32_t _calendar_date = (tmInfo.tm_year + 1900) * 10000 + (tmInfo.tm_mon + 1) * 100 + tmInfo.tm_mday;

    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    pref.putInt(PREF_SI_CAL_DATE, _calendar_date);
    pref.end();

    display.powerOff();
    display.hibernate();
    Serial.println("[Task] screen update end...");

    _screen_status = 1;
    SCREEN_HANDLER = NULL;
    vTaskDelete(NULL);
}

void si_screen() {
    _screen_status = 0;
    si_calendar(); // 准备日历数据

    if (si_calendar_status() == 2) {
        Serial.println("ERR: System time prepare failed.");
        _screen_status = 2;
        return;
    }

    if (SCREEN_HANDLER != NULL) {
        vTaskDelete(SCREEN_HANDLER);
    }
    xTaskCreate(task_screen, "Screen", 4096, NULL, 2, &SCREEN_HANDLER);
}

int si_screen_status() {
    return _screen_status;
}

void print_status() {
    Serial.printf("Weather: %d\n", weather_status());
    Serial.printf("Calendar: %d\n", si_calendar_status());
    Serial.printf("Screen: %d\n", si_screen_status());
}



void si_warning(const char* str) {
    Serial.println("Screen warning...");
    display.init(115200);          // 串口使能 初始化完全刷新使能 复位时间 ret上拉使能
    display.setRotation(ROTATION); // 设置屏幕旋转1和3是横向  0和2是纵向
    u8g2Fonts.begin(display);

    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);
    do {
        u8g2Fonts.setFontMode(1);
        u8g2Fonts.setFontDirection(0);
        u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
        u8g2Fonts.setForegroundColor(GxEPD_BLACK);

        u8g2Fonts.setFont(u8g2_font_open_iconic_all_4x_t);
        int space = 8;
        int w = u8g2Fonts.getUTF8Width("\u0118") + space;
        u8g2Fonts.setFont(FONT_TEXT);
        w += u8g2Fonts.getUTF8Width(str);

        u8g2Fonts.setForegroundColor(GxEPD_RED);
        u8g2Fonts.setFont(u8g2_font_open_iconic_all_4x_t);
        u8g2Fonts.setCursor((display.width() - w) / 2, (display.height() + 32) / 2);
        u8g2Fonts.print("\u0118");

        u8g2Fonts.setForegroundColor(GxEPD_BLACK);
        u8g2Fonts.setCursor(u8g2Fonts.getCursorX() + space, u8g2Fonts.getCursorY() - 8);
        u8g2Fonts.setFont(FONT_TEXT);
        u8g2Fonts.print(str);
    } while (display.nextPage());

    display.powerOff();
    display.hibernate();
}