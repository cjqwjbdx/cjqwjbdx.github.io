#ifndef ___PREFERENCE_H__
#define ___PREFERENCE_H__

#include <Preferences.h>
#define PREF_NAMESPACE "J_CALENDAR"

// Preferences KEY定义
// !!!preferences key限制15字符
#define PREF_SI_CAL_DATE "SI_CAL_DATE" // 屏幕当前显示的日期
#define PREF_SI_WEEK_1ST "SI_WEEK_1ST" // 每周第一天，0: 周日（默认），1:周一
#define PREF_SI_TYPE "SI_TYPE" // 屏幕显示类型

#define PREF_WEATHER_TYPE "WEATHER_TYPE" // 0: 每日天气，1: 实时天气
#define PREF_DEFAULT_CITY "DEFAULT_CITY" // IP定位失败时的默认城市
#define PREF_API_KEY "WEATHER_API_KEY" // 天气API密钥
#define PREF_WEEK_INFO "WEEK_INFO" // 0: 定位位置，1: 刷新时间，2: 系统版本
#define PREF_CD_DAY_DATE "CD_DAY_DATE" // 倒计日
#define PREF_CD_DAY_LABLE "CD_DAY_LABLE" // 倒计日名称
#define PREF_TAG_DAYS "TAG_DAYS" // tag day
#define PREF_STUDY_SCHEDULE "STUDY_SCH" // 课程表

// 小部件配置
#define PREF_WIDGETS_GOLD "WIDGETS_GOLD" // 是否显示黄金价格（"0" 或 "1"）
#define PREF_WIDGETS_GOLD_PRICE "WIDGETS_GOLD_P" // 缓存黄金价格文本

// 假期信息，tm年，假期日(int8)，假期日(int8)...
#define PREF_HOLIDAY "HOLIDAY"

#endif
