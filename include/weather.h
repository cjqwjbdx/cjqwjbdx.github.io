#ifndef __WEATHER_H__
#define __WEATHER_H__

#include <API.hpp>

int8_t weather_type();
int8_t weather_status();
Weather* weather_data_now();
DailyForecast* weather_data_daily();
void weather_exec(int status = 0);
void weather_stop();

// 获取定位城市
String weather_location_city();
// 获取第几周旁边显示的信息类型 (0:定位位置, 1:刷新时间, 2:系统版本)
int8_t week_info_type();
// 获取API更新时间 (格式: HH:MM)
String weather_update_time();

#endif