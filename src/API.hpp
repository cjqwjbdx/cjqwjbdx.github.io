#ifndef __API_HPP__
#define __API_HPP__

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <esp_http_client.h>

#include <ArduinoUZlib.h> // 解压gzip

struct Weather {
    String time;
    int8_t temp;
    int8_t humidity;
    int16_t wind360;
    String windDir;
    int8_t windScale;
    uint8_t windSpeed;
    uint16_t icon;
    String text;
    String updateTime;
    uint16_t now_weather_id;  // 新增：当前天气ID（白天/晚上）
};

struct DailyWeather {
    String date;
    String sunrise;
    String sunset;
    String moonPhase;
    uint16_t moonPhaseIcon;
    int8_t tempMax;
    int8_t tempMin;
    int8_t humidity;
    uint16_t iconDay;
    String textDay;
    uint16_t iconNight;
    String textNight;
    int16_t wind360Day;
    String windDirDay;
    int8_t windScaleDay;
    uint8_t windSpeedDay;
    int16_t wind360Night;
    String windDirNight;
    int8_t windScaleNight;
    uint8_t windSpeedNight;
};

struct HourlyForecast {
    Weather* weather;
    uint8_t length;
    uint8_t interval;
};

struct DailyForecast {
    DailyWeather* weather;
    uint8_t length;
    String updateTime;
};

struct ThreeDayForecast {
    Weather* day1;
    Weather* day2;
    Weather* day3;
    uint8_t length;
    String updateTime;
};

struct Hitokoto {
    String sentence;
    String from;
    String from_who;
};

struct Bilibili {
    uint64_t follower;
    uint64_t view;
    uint64_t likes;
};

struct GoldPrice {
    String price_text;
};

struct LocationInfo {
    String ip;
    String city;
    String district;
    String province;
};
template<uint8_t MAX_RETRY = 3>
class API {
    using callback = std::function<bool(JsonDocument&)>;
    using precall = std::function<void()>;

private:
    HTTPClient http;
    WiFiClientSecure wifiClient;

    bool getRestfulAPI(String url, callback cb, precall pre = precall()) {
        // Serial.printf("Request Url: %s\n", url.c_str());
        JsonDocument doc;

        for (uint8_t i = 0; i < MAX_RETRY; i++) {
            bool shouldRetry = false;
            if (http.begin(wifiClient, url)) {
                if (pre) pre();
                // Serial.printf("Before GET\n");
                int httpCode = http.GET();
                // Serial.printf("GET %d\n", httpCode);
                if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_NOT_MODIFIED) {
                    bool isGzip = false;
                    int headers = http.headers();
                    for (int j = 0; j < headers; j++) {
                        String headerName = http.headerName(j);
                        String headerValue = http.header(j);
                        // Serial.println(headerName + ": " + headerValue);
                        if (headerName.equalsIgnoreCase("Content-Encoding") && headerValue.equalsIgnoreCase("gzip")) {
                            isGzip = true;
                            break;
                        }
                    }
                    String s = http.getString();
                    DeserializationError error;
                    if (isGzip) {
                        // gzip解压缩
                        uint8_t* outBuf = NULL;
                        uint32_t outLen = 0;
                        ArduinoUZlib::decompress((uint8_t*)s.c_str(), (uint32_t)s.length(), outBuf, outLen);
                        error = deserializeJson(doc, (char*)outBuf, outLen);
                        free(outBuf);
                    } else {
                        error = deserializeJson(doc, s);
                    }

                    if (!error) {
                        wifiClient.flush();
                        http.end();
                        return cb(doc);
                    } else {
                        Serial.print(F("Parse JSON failed, error: "));
                        Serial.println(error.c_str());
                        shouldRetry = error == DeserializationError::IncompleteInput;
                    }
                } else {
                    Serial.print(F("Get failed, error: "));
                    if (httpCode < 0) {
                        Serial.println(http.errorToString(httpCode));
                        shouldRetry = httpCode == HTTPC_ERROR_CONNECTION_REFUSED || httpCode == HTTPC_ERROR_CONNECTION_LOST || httpCode == HTTPC_ERROR_READ_TIMEOUT;
                    } else {
                        Serial.println(httpCode);
                    }
                }
                wifiClient.flush();
                http.end();
            } else {
                Serial.println(F("Unable to connect"));
            }
            if (!shouldRetry) break;
            Serial.println(F("Retry after 10 second"));
            delay(5000);
        }
        return false;
    }

public:
    API() {
        // http.setTimeout(10000);
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        const char* encoding = "Content-Encoding";
        const char* headerKeys[1] = {};
        headerKeys[0] = encoding;
        http.collectHeaders(headerKeys, 1);

        wifiClient.setInsecure();
    }

    ~API() {}

    // 获取 HTTPClient
    HTTPClient& httpClient() {
        return http;
    }

    // 一言: https://developer.hitokoto.cn/sentence/
    bool getHitokoto(Hitokoto& result) {
        return getRestfulAPI("https://v1.hitokoto.cn/?max_length=15", [&result](JsonDocument& json) {
            result.sentence = json["hitokoto"].as<const char*>();
            result.from = json["from"].as<const char*>();
            result.from_who = json["from_who"].as<const char*>();
            return true;
            });
    }

    // B站粉丝
    bool getFollower(Bilibili& result, uint32_t uid) {
        return getRestfulAPI("https://api.bilibili.com/x/relation/stat?vmid=" + String(uid), [&result](JsonDocument& json) {
            if (json["code"] != 0) {
                Serial.print(F("Get bilibili follower failed, error: "));
                Serial.println(json["message"].as<const char*>());
                return false;
            }
            result.follower = json["data"]["follower"];
            return true;
            });
    }

    // B站总播放量和点赞数
    bool getLikes(Bilibili& result, uint32_t uid, const char* cookie) {
        return getRestfulAPI(
            "https://api.bilibili.com/x/space/upstat?mid=" + String(uid), [&result](JsonDocument& json) {
                if (json["code"] != 0) {
                    Serial.print(F("Get bilibili likes failed, error: "));
                    Serial.println(json["message"].as<const char*>());
                    return false;
                }
                result.view = json["data"]["archive"]["view"];
                result.likes = json["data"]["likes"];
                return true;
            },
            [this, &cookie]() {
                http.addHeader("Cookie", String("SESSDATA=") + cookie + ";");
            });
    }

    bool getGoldPrice(GoldPrice& result) {
        return getRestfulAPI("https://api.freejk.com/shuju/jinjia/", [&result](JsonDocument& json) {
            // 检查状态码
            if (!json["status"] || strcmp(json["status"].as<const char*>(), "success") != 0) {
                Serial.println(F("Get gold price failed: API status not success"));
                return false;
            }
            // 检查 data 字段是否存在
            if (!json["data"]) {
                Serial.println(F("Get gold price failed: data field not found"));
                return false;
            }
            // 从嵌套的 data 对象中获取价格
            result.price_text = json["data"]["price_text"].as<const char*>();
            Serial.printf("[Gold API] Raw price_text: %s\n", result.price_text.c_str());
            return true;
            });
    }

    // XXAPI IP定位接口
    bool getXXAPILocation(LocationInfo& result) {
        return getRestfulAPI("https://v2.xxapi.cn/api/ip", [&result](JsonDocument& json) {
            if (json["code"].as<int>() != 200) {
                Serial.print(F("Get IP location failed, error: "));
                Serial.println(json["msg"].as<const char*>());
                return false;
            }
            if (!json["data"]) {
                Serial.println(F("Get IP location failed: data field not found"));
                return false;
            }

            // 获取IP
            result.ip = json["data"]["ip"].isNull() ? "" : json["data"]["ip"].as<const char*>();

            // 获取完整地址
            String fullAddress = json["data"]["address"].isNull() ? "" : json["data"]["address"].as<const char*>();
            result.district = fullAddress;

            // 从address字段解析
            if (!fullAddress.isEmpty()) {
                // 直接使用完整地址作为天气查询参数
                // 天气API可以接受"四川省成都市双流区"这样的完整地址
                result.province = "";
                result.city = "";

                // 将完整地址用于天气查询，不再拆分
                Serial.printf("[Debug] Using full address: %s (len=%d)\n", fullAddress.c_str(), fullAddress.length());

            } else {
                result.city = "";
                result.province = "";
                result.district = "";
            }

            Serial.printf("[IP Location] IP: %s, Full Address: %s\n",
                         result.ip.c_str(), result.district.c_str());
            return true;
        });
    }


    // UAPI 天气接口（返回当前天气和forecast）
    bool getUapiWeatherForecast(DailyForecast& result, Weather& currentWeather, const char* city, const char* apiKey = "") {
        String url = "https://uapis.cn/api/v1/misc/weather?city=" + String(city) + "&forecast=true";
        if (strlen(apiKey) > 0) {
            url += "&api_key=" + String(apiKey);
        }

        return getRestfulAPI(
            url, [&result, &currentWeather, city](JsonDocument& json) {
                Serial.printf("[Weather] Raw JSON response: %s\n", json.as<String>().c_str());

                // UAPI返回格式: {"province":"四川省","city":"成都市","weather":"阴",...}

                // 填充当前天气数据
                if (!json["report_time"].isNull()) {
                    result.updateTime = json["report_time"].as<const char*>();
                    currentWeather.updateTime = result.updateTime;
                    currentWeather.time = result.updateTime;
                    Serial.printf("[Weather] Report time: %s\n", result.updateTime.c_str());
                }

                currentWeather.text = json["weather"].isNull() ? "未知" : json["weather"].as<const char*>();
                currentWeather.temp = json["temperature"].isNull() ? 0 : json["temperature"].as<int>();
                currentWeather.humidity = json["humidity"].isNull() ? 0 : json["humidity"].as<int>();
                currentWeather.windDir = json["wind_direction"].isNull() ? "" : json["wind_direction"].as<const char*>();

                // 处理wind_power字段（可能是字符串"2"或数字2）
                if (json["wind_power"].is<int>()) {
                    currentWeather.windScale = json["wind_power"].as<int>();
                } else {
                    String windPowerStr = json["wind_power"].isNull() ? "0" : json["wind_power"].as<const char*>();
                    currentWeather.windScale = windPowerStr.toInt();
                }

                currentWeather.icon = json["weather_icon"].isNull() ? 0 : json["weather_icon"].as<uint16_t>();
                currentWeather.wind360 = 0;
                currentWeather.windSpeed = 0;

                // 根据天气类型和时间设置now_weather_id（使用UAPI天气代码表）
                String weather_text = json["weather"].isNull() ? "" : json["weather"].as<const char*>();
                String report_time = json["report_time"].isNull() ? "" : json["report_time"].as<const char*>();
                int hour = 12; // 默认中午时间

                if (!report_time.isEmpty()) {
                    hour = report_time.substring(11, 13).toInt();
                }

                bool is_night = (hour < 6 || hour >= 18); // 晚上6点到早上6点

                // 使用查表方式映射天气文本到白天/夜间图标代码
                // 严格对照和风天气图标代码表
                struct WMap { const char* txt; uint16_t day; uint16_t night; };
                static const WMap wmap[] = {
                    // 晴/多云类 100-104, 150-153
                    {"晴", 100, 150},
                    {"晴朗", 100, 150},
                    {"多云", 101, 151},
                    {"大部分地区多云", 101, 151},
                    {"少云", 102, 152},
                    {"晴间多云", 103, 153},
                    {"局部多云", 103, 153},
                    {"阴", 104, 104},

                    // 雨类 300-318, 350-351, 399
                    {"阵雨", 300, 350},
                    {"强阵雨", 301, 351},
                    {"雷阵雨", 302, 302},
                    {"强雷阵雨", 303, 303},
                    {"雷阵雨伴有冰雹", 304, 304},
                    {"小雨", 305, 305},
                    {"中雨", 306, 306},
                    {"大雨", 307, 307},
                    {"极端降雨", 308, 308},
                    {"毛毛雨/细雨", 309, 309},
                    {"暴雨", 310, 310},
                    {"大暴雨", 311, 311},
                    {"特大暴雨", 312, 312},
                    {"冻雨", 313, 313},
                    {"小到中雨", 314, 314},
                    {"中到大雨", 315, 315},
                    {"大到暴雨", 316, 316},
                    {"暴雨到大暴雨", 317, 317},
                    {"大暴雨到特大暴雨", 318, 318},
                    {"雨", 399, 399},

                    // 雪类 400-410, 499
                    {"小雪", 400, 400},
                    {"中雪", 401, 401},
                    {"大雪", 402, 402},
                    {"暴雪", 403, 403},
                    {"雨夹雪", 404, 404},
                    {"阵雪", 405, 405},
                    {"雨雪天气", 406, 406},
                    {"阵雨夹雪", 407, 407},
                    {"小到中雪", 408, 408},
                    {"中到大雪", 409, 409},
                    {"大到暴雪", 410, 410},
                    {"雪", 499, 499},

                    // 特殊雪类 456-457 (仅白天)
                    {"阵雨夹雪(456)", 456, 456},
                    {"阵雪(457)", 457, 457},

                    // 雾/霾/沙尘 500-515
                    {"雾", 500, 500},
                    {"薄雾", 501, 501},
                    {"霾", 502, 502},
                    {"扬沙", 503, 503},
                    {"浮尘", 504, 504},
                    {"沙尘暴", 507, 507},
                    {"强沙尘暴", 508, 508},
                    {"浓雾", 509, 509},
                    {"强浓雾", 510, 510},
                    {"中度霾", 511, 511},
                    {"重度霾", 512, 512},
                    {"严重霾", 513, 513},
                    {"大雾", 514, 514},
                    {"特强浓雾", 515, 515},

                    // 其他 900-902, 999
                    {"热", 900, 900},
                    {"冷", 901, 901},
                    {"未知", 999, 999},
                };

                // 查表（精确匹配），如未找到则回退到 icon 或按 day/night 选择
                uint16_t mapped = 0;
                short found = 0;
                for (size_t i = 0; i < sizeof(wmap)/sizeof(wmap[0]); ++i) {
                    if (currentWeather.icon ==wmap[i].day || currentWeather.icon == wmap[i].night) {
                        found = 2;
                        break;
                    }
                    
                    if (weather_text.equals(wmap[i].txt)) {
                        mapped = is_night ? wmap[i].night : wmap[i].day;
                        found = 1;
                        break;
                    }
                }
                if (found==1) {
                    currentWeather.now_weather_id = mapped;
                }else if(found==2){
                    currentWeather.now_weather_id = currentWeather.icon; 
                }else currentWeather.now_weather_id = 999; // 未匹配到，返回未知
                

                Serial.printf("[Weather Current] %s, %d°C, 湿度%d%%, %s%d级, 代码:%d, 图标ID:%d\n",
                             currentWeather.text.c_str(), currentWeather.temp, currentWeather.humidity,
                             currentWeather.windDir.c_str(), currentWeather.windScale,
                             currentWeather.icon, currentWeather.now_weather_id);

                // 解析forecast数组
                if (!json["forecast"].isNull()) {
                    JsonArray forecastArray = json["forecast"];
                    uint8_t length = min((uint8_t)forecastArray.size(), result.length);

                    for (uint8_t i = 0; i < length; i++) {
                        DailyWeather& weather = result.weather[i];
                        JsonObject day = forecastArray[i];

                        // 字段名: temp_max, temp_min, weather_day, wind_dir_day, wind_scale_day
                        weather.date = day["date"].isNull() ? "" : day["date"].as<const char*>();
                        weather.tempMax = day["temp_max"].isNull() ? 0 : day["temp_max"].as<int>();
                        weather.tempMin = day["temp_min"].isNull() ? 0 : day["temp_min"].as<int>();
                        weather.humidity = day["humidity"].isNull() ? 0 : day["humidity"].as<int>();
                        weather.textDay = day["weather_day"].isNull() ? "未知" : day["weather_day"].as<const char*>();
                        
                        // 查表获取白天图标代码
                        String weather_day_text = day["weather_day"].isNull() ? "" : day["weather_day"].as<const char*>();
                        weather.iconDay = 999; // 默认未知
                        for (size_t j = 0; j < sizeof(wmap)/sizeof(wmap[0]); j++) {
                            if (weather_day_text.equals(wmap[j].txt)) {
                                weather.iconDay = wmap[j].day;
                                break;
                            }
                        }
                        
                        weather.windDirDay = day["wind_dir_day"].isNull() ? "" : day["wind_dir_day"].as<const char*>();

                        // wind_scale_day可能是"1-3"这样的字符串，取第一个数字
                        String windScaleStr = day["wind_scale_day"].isNull() ? "0" : day["wind_scale_day"].as<const char*>();
                        int firstDigit = windScaleStr.toInt();
                        weather.windScaleDay = firstDigit > 0 ? firstDigit : 0;

                        Serial.printf("[Weather Forecast Day %d] %s | 温度:%d~%d°C | 湿度:%d%% | %s | %s%d级 | 代码:%d\n",
                                     i + 1, weather.date.c_str(), weather.tempMin, weather.tempMax,
                                     weather.humidity, weather.textDay.c_str(),
                                     weather.windDirDay.c_str(), weather.windScaleDay, weather.iconDay);
                    }

                    result.length = length;
                } else {
                    result.length = 0;
                }

                Serial.printf("[Weather Forecast] Total: %d days forecast for %s\n", result.length, city);
                return true;
            });
    }
};
#endif  // __API_HPP__
