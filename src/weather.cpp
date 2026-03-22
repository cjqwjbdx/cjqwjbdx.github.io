#include "weather.h"

#include <_preference.h>

TaskHandle_t WEATHER_HANDLER;

int8_t _weather_status = -1;
Weather _weather_now = {};
// 为逐天预报分配三天的存储空间，以便 API 填充最多三天的数据
DailyWeather dailyWeather[3] = {};
DailyForecast _daily_forecast = {
    .weather = dailyWeather,
    .length = 3
};

String _detected_city = "";  // 存储定位到的城市

int8_t weather_status() {
    return _weather_status;
}

String weather_location_city() {
    return _detected_city;
}

String weather_update_time() {
    // 优先使用实时天气的更新时间，如果没有则使用预报的更新时间
    if (!_weather_now.updateTime.isEmpty()) {
        // updateTime格式: "2025-02-10T21:00:00+08:00" 或 "2025-02-10 21:00:00"
        String timeStr = _weather_now.updateTime;
        int hour = 0, minute = 0;

        // 尝试提取时间部分 (HH:MM)
        int tPos = timeStr.indexOf('T');
        if (tPos < 0) tPos = timeStr.indexOf(' ');

        if (tPos >= 0 && tPos + 5 < timeStr.length()) {
            hour = timeStr.substring(tPos + 1, tPos + 3).toInt();
            minute = timeStr.substring(tPos + 4, tPos + 6).toInt();
        }

        char buf[8];
        snprintf(buf, sizeof(buf), "%02d:%02d", hour, minute);
        return String(buf);
    } else if (!_daily_forecast.updateTime.isEmpty()) {
        String timeStr = _daily_forecast.updateTime;
        int hour = 0, minute = 0;

        int tPos = timeStr.indexOf('T');
        if (tPos < 0) tPos = timeStr.indexOf(' ');

        if (tPos >= 0 && tPos + 5 < timeStr.length()) {
            hour = timeStr.substring(tPos + 1, tPos + 3).toInt();
            minute = timeStr.substring(tPos + 4, tPos + 6).toInt();
        }

        char buf[8];
        snprintf(buf, sizeof(buf), "%02d:%02d", hour, minute);
        return String(buf);
    }
    return "";
}

int8_t week_info_type() {
    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    String type = pref.getString(PREF_WEEK_INFO, "0");
    pref.end();
    return type.toInt();
}
int8_t weather_type() {
    // 从Preferences读取天气类型设置
    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    String type = pref.getString(PREF_WEATHER_TYPE, "0");
    pref.end();
    return type == "1" ? 1 : 0;
}
Weather* weather_data_now() {
    return &_weather_now;
}
DailyForecast* weather_data_daily() {
    return &_daily_forecast;
}

void task_weather(void* param) {
    Serial.println("[Task] get weather begin...");

    API<> api;
    String detectedCity;

    // 第一步：IP定位获取城市
    LocationInfo location;
    bool locationSuccess = api.getXXAPILocation(location);

    Preferences pref;
    pref.begin(PREF_NAMESPACE);

    if (locationSuccess) {
        // 直接使用完整地址查询天气，不再拆分
        detectedCity = location.district; // 完整地址如"中国四川省成都市双流区"
        _detected_city = location.district; // 保存到全局变量用于显示
        pref.putString("location_failed", "false");
        Serial.printf("========== 位置信息 ==========\n");
        Serial.printf("IP: %s\n", location.ip.c_str());
        Serial.printf("完整地址: %s\n", location.district.c_str());
        Serial.printf("查询天气: %s\n", detectedCity.c_str());
        Serial.printf("==============================\n");
    } else {
        // IP定位失败，使用配置的默认城市
        detectedCity = pref.getString(PREF_DEFAULT_CITY, "北京");
        _detected_city = detectedCity; // 保存默认城市
        pref.putString("location_failed", "true");
        Serial.printf("========== 位置信息 ==========\n");
        Serial.printf("IP定位失败\n");
        Serial.printf("使用默认城市: %s\n", detectedCity.c_str());
        Serial.printf("==============================\n");
    }

    pref.end();

    detectedCity.replace(" ", "%20");  // URL编码空格
    Serial.printf("请求天气城市: %s\n", detectedCity.c_str());

    // 读取API密钥
    Preferences pref2;
    pref2.begin(PREF_NAMESPACE);
    String apiKey = pref2.getString(PREF_API_KEY, "");
    pref2.end();

    // 第二步：获取天气预报（forecast=true），同时获取当前天气
    Serial.printf("\n========== 开始获取天气 ==========\n");
    bool forecastSuccess = api.getUapiWeatherForecast(_daily_forecast, _weather_now, detectedCity.c_str(), apiKey.c_str());

    if (forecastSuccess) {
        _weather_status = 1;
        Serial.printf("========== 天气获取完成 ==========\n");
        Serial.printf("状态: 成功\n");
        Serial.printf("城市: %s\n", detectedCity.c_str());
        Serial.printf("预报天数: %d天\n", _daily_forecast.length);
    } else {
        _weather_status = 2;
        Serial.printf("========== 天气获取失败 ==========\n");
    }

    Serial.println("[Task] get weather end...");
    WEATHER_HANDLER = NULL;
    vTaskDelete(NULL);
}

void weather_exec(int status) {
    _weather_status = status;
    if (status > 0) {
        return;
    }

    if (!WiFi.isConnected()) {
        _weather_status = 2;
        return;
    }

    if (WEATHER_HANDLER != NULL) {
        vTaskDelete(WEATHER_HANDLER);
        WEATHER_HANDLER = NULL;
    }
    xTaskCreate(task_weather, "WeatherData", 1024 * 8, NULL, 2, &WEATHER_HANDLER);
}

void weather_stop() {
    if (WEATHER_HANDLER != NULL) {
        vTaskDelete(WEATHER_HANDLER);
        WEATHER_HANDLER = NULL;
    }
    _weather_status = 2;
}
