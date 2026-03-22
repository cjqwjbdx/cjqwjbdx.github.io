#include <Arduino.h>
#include <ArduinoJson.h>

#include <WiFiManager.h>

#include "esp_sleep.h"

#include <wiring.h>

#include "battery.h"

#include "led.h"
#include "_sntp.h"
#include "weather.h"
#include "screen_ink.h"
#include "_preference.h"
#include "gold.h"
// #include "ble_config.h"  // 暂时注释 BLE

#include "version.h"

#include "OneButton.h"
OneButton button(KEY_M, true);

void IRAM_ATTR checkTicks() {
    button.tick();
}

WiFiManager wm;
WiFiManagerParameter para_weather_type("weather_type", "天气类型 (0:每日预报显示最高最低温, 1:实时天气显示当前温湿度)", "0", 2, "pattern='[0-1]'");
WiFiManagerParameter para_default_city("default_city", "默认城市（IP定位失败时使用）", "北京", 32);
WiFiManagerParameter para_week_info("week_info", "第几周右边显示 (0:定位位置, 1:刷新时间, 2:系统版本)", "0", 2, "pattern='[0-2]'");
WiFiManagerParameter para_cd_day_label("cd_day_label", "倒数日（4字以内）", "", 10); //     倒数日
WiFiManagerParameter para_cd_day_date("cd_day_date", "日期（yyyyMMdd）", "", 8, "pattern='\\d{8}'"); //     城市code
WiFiManagerParameter para_tag_days("tag_days", "日期Tag（yyyyMMddx，详见README）", "", 30); //     日期Tag
WiFiManagerParameter para_si_week_1st("si_week_1st", "每周起始（0:周日，1:周一）", "0", 2, "pattern='\\[0-1]{1}'"); //     每周第一天
WiFiManagerParameter para_study_schedule("study_schedule", "课程表", "0", 4000, "pattern='\\[0-9]{3}[;]$'"); //     每周第一天
WiFiManagerParameter para_widgets_gold("widgets_gold", "显示黄金价格（0:否，1:是）", "0", 2, "pattern='\\[0-1]{1}'"); //     显示黄金价格
void print_wakeup_reason() {
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
        Serial.println("Wakeup caused by external signal using RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
    {
        Serial.println("Wakeup caused by external signal using RTC_CNTL");
        uint64_t status = esp_sleep_get_ext1_wakeup_status();
        if (status == 0) {
            Serial.println(" *None of the configured pins woke us up");
        } else {
            Serial.print(" *Wakeup pin mask: ");
            Serial.printf("0x%016llX\r\n", status);
            for (int i = 0; i < 64; i++) {
                if ((status >> i) & 0x1) {
                    Serial.printf("  - GPIO%d\r\n", i);
                }
            }
        }
        break;
    }
    case ESP_SLEEP_WAKEUP_TIMER:
        Serial.println("Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        Serial.println("Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        Serial.println("Wakeup caused by ULP program");
        break;
    default:
        Serial.printf("Wakeup was not caused by deep sleep.\r\n");
    }
}

void buttonClick(void* oneButton);
void buttonDoubleClick(void* oneButton);
void buttonLongPressStop(void* oneButton);
void go_sleep();
void saveParamsCallback();
void preSaveParamsCallback();

unsigned long _idle_millis;
unsigned long TIME_TO_SLEEP = 180 * 1000;

bool _wifi_flag = false;
unsigned long _wifi_failed_millis;
void setup() {
    delay(10);
    Serial.begin(115200);
    Serial.println(".");
    print_wakeup_reason();
    Serial.println("\r\n\r\n");
    delay(10);


    Serial.printf("***********************\r\n");
    Serial.printf("      WuJinOS\r\n");
    Serial.printf("    version: %s\r\n", J_VERSION);
    Serial.printf("***********************\r\n\r\n");
    Serial.printf("Copyright © 2022-2026 wjbdx. All Rights Reserved.\r\n\r\n");

    led_init();
    led_on();
    delay(100);
    int voltage = readBatteryVoltage();
    Serial.printf("Battery: %d mV\r\n", voltage);
    if(voltage < 2500) {
        Serial.println("[INFO]电池损坏或无ADC电路。");
    } else if(voltage < 3000) {
        Serial.println("[WARN]电量低于3v，系统休眠。");
        go_sleep();
    } else if (voltage < 3300) {
        // 低于3.3v，电池电量用尽，屏幕给警告，然后关机。
        Serial.println("[WARN]电量低于3.3v，警告并系统休眠。");
        si_warning("电量不足，请充电！");
        go_sleep();
    } else if (voltage > 4400) {
        Serial.println("[INFO]未接电池。");
    }

    button.setClickMs(300);
    button.setPressMs(3000); // 设置长按的时长
    button.attachClick(buttonClick, &button);
    button.attachDoubleClick(buttonDoubleClick, &button);
    // button.attachMultiClick()
    button.attachLongPressStop(buttonLongPressStop, &button);
    attachInterrupt(digitalPinToInterrupt(KEY_M), checkTicks, CHANGE);

    // 初始化黄金价格模块
    gold_init();

    // 不在这里初始化蓝牙，而是在配置模式时再初始化
    // ble_config_init();  // 暂时注释 BLE

    Serial.println("Wm begin...");
    led_fast();
    wm.setHostname("WuJinOS");
    wm.setEnableConfigPortal(true);
    wm.setConnectTimeout(10);

    // 先尝试自动连接（使用保存的 WiFi 配置）
    if (wm.autoConnect("WuJinOS")) {
        Serial.println("Connect OK.");
        led_on();
        _wifi_flag = true;
        
        // WiFi连接成功后立即获取金价数据
        if (gold_status() == 0) {
            gold_update();
        }
    } else {
        Serial.println("Connect failed, starting config portal...");
        
        // 配置门户设置
        wm.setTitle("WuJinOS");
        wm.addParameter(&para_si_week_1st);
        wm.addParameter(&para_weather_type);
        wm.addParameter(&para_default_city);
        wm.addParameter(&para_week_info);
        wm.addParameter(&para_cd_day_label);
        wm.addParameter(&para_cd_day_date);
        wm.addParameter(&para_tag_days);
        wm.addParameter(&para_study_schedule);
        wm.addParameter(&para_widgets_gold);
        std::vector<const char*> menu = { "wifi","param","update","sep","info","restart","exit" };
        wm.setMenu(menu);
        wm.setConfigPortalBlocking(false);
        wm.setBreakAfterConfig(true);
        wm.setPreSaveParamsCallback(preSaveParamsCallback);
        wm.setSaveParamsCallback(saveParamsCallback);
        wm.setSaveConnect(false);
        wm.startConfigPortal("WuJinOS");
        
        led_config();
        _idle_millis = millis();
        _wifi_flag = false;
        _wifi_failed_millis = millis();
        led_slow();
        _sntp_exec(2);
        weather_exec(2);
        gold_update(2);
        WiFi.mode(WIFI_OFF);
        Serial.println("Wifi closed.");
    }
}

/**
 * 处理各个任务
 * 1. sntp同步
 *      前置条件：Wifi已连接
 * 2. 刷新日历
 *      前置条件：sntp同步完成（无论成功或失败）
 * 3. 刷新天气信息
 *      前置条件：wifi已连接
 * 4. 系统配置
 *      前置条件：无
 * 5. 休眠
 *      前置条件：所有任务都完成或失败，
 */
/**
 * @brief 主循环函数，处理设备的主要逻辑流程
 * 
 * 该函数负责协调设备的各种任务，包括：
 * - 按钮事件处理
 * - WiFi管理
 * - SNTP时间同步
 * - 黄金价格更新
 * - 天气信息获取
 * - 屏幕刷新
 * - 系统休眠
 * 
 * 函数按照以下顺序执行任务：
 * 1. 处理按钮事件（单击刷新、双击配置、长按重启）
 * 2. 检查并执行SNTP时间同步
 * 3. 更新黄金价格（如果未初始化）
 * 4. 获取天气信息（如果未初始化）
 * 5. 在满足条件时刷新屏幕并关闭WiFi
 * 6. 每分钟更新时间显示
 * 7. 每5秒输出调试信息
 * 8. 在适当条件下进入休眠模式
 * 
 * @note 前置条件：
 * - 对于SNTP同步：需要WiFi已连接
 * - 对于屏幕刷新：需要SNTP、天气和金价任务已完成
 * - 对于休眠：需要屏幕刷新完成或配置超时
 * 
 * @note 定时任务：
 * - 每分钟检查时间变化并更新显示
 * - 每5秒输出一次系统状态调试信息
 * 
 * @note 休眠条件：
 * - 非配置状态下屏幕刷新完成
 * - 配置状态下超过空闲时间(TIME_TO_SLEEP)
 * 
 * @see button.tick()
 * @see wm.process()
 * @see _sntp_status()
 * @see _sntp_exec()
 * @see gold_status()
 * @see gold_update()
 * @see weather_status()
 * @see weather_exec()
 * @see si_screen_status()
 * @see si_screen()
 * @see refresh_time_info()
 * @see update_time_display()
 * @see go_sleep()
 */
void loop() {
    static uint32_t lastMinuteCheck = 0;

    button.tick(); // 单击，刷新页面；双击，打开配置；长按，重启
    wm.process();
    // ble_config_loop(); // 暂时注释 BLE

    // 前置任务：wifi已连接
    // sntp同步
    if (_sntp_status() == -1) {
        _sntp_exec();
    }
    // 如果是定时器唤醒，并且接近午夜（23:50之后），则直接休眠
    if (_sntp_status() == SYNC_STATUS_TOO_LATE) {
        go_sleep();
    }
    // 前置任务：wifi已连接
    // 更新黄金价格（在天气获取之前执行）
    // 只有在状态为-1（未初始化）时才执行更新
    if (gold_status() == -1) {
        gold_update();
    }

    // 获取Weather信息
    if (weather_status() == -1) {
        weather_exec();
    }

    // 刷新日历
    // 前置任务：sntp、weather
    // 执行条件：屏幕状态为待处理，并且金价任务已处理（成功或失败）
    if (_sntp_status() > 0 && weather_status() > 0 && gold_status() > 0 && si_screen_status() == -1) {
        // 数据获取完毕后，先刷新屏幕（包括可能的金价请求），然后关闭Wifi以省电
        si_screen();
        if (!wm.getConfigPortalActive()) {
            WiFi.mode(WIFI_OFF);
            Serial.println("Wifi closed after data fetch.");
        }
    }
    
// 每分钟检查一次时间变化并执行局刷更新
    static uint32_t lastTimeDebug = 0;
    // 时间显示已禁用
    // if (si_screen_status() > 0 && millis() - lastMinuteCheck > 60000) {
    //     lastMinuteCheck = millis();
    //     Serial.printf("[MAIN] Time update triggered, screen_status=%d\n", si_screen_status());
    //     refresh_time_info();
    //     update_time_display();
    // }
    // 每5秒输出一次调试信息
    if (millis() - lastTimeDebug > 5000) {
        lastTimeDebug = millis();
        Serial.printf("[MAIN] Loop running, screen_status=%d, heap=%d\n", 
                      si_screen_status(), ESP.getFreeHeap());
    }

    // 休眠
    // 前置条件：屏幕刷新完成（或成功）

    // 未在配置状态，且屏幕刷新完成，进入休眠
    if (!wm.getConfigPortalActive() && si_screen_status() > 0) {
        go_sleep();
    }
    // 配置状态下，
    if (wm.getConfigPortalActive() && millis() - _idle_millis > TIME_TO_SLEEP) {
        go_sleep();
    }

    delay(10);
}


// 刷新页面
void buttonClick(void* oneButton) {
    Serial.println("Button click.");
    if (wm.getConfigPortalActive()) {
        Serial.println("In config status.");
    } else {
        Serial.println("Refresh screen manually.");
        Preferences pref;
        pref.begin(PREF_NAMESPACE);
        int _si_type = pref.getInt(PREF_SI_TYPE);
        pref.putInt(PREF_SI_TYPE, _si_type == 0 ? 1 : 0);
        pref.end();
        si_screen();
    }
}

void saveParamsCallback() {
    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    pref.putString(PREF_WEATHER_TYPE, strcmp(para_weather_type.getValue(), "1") == 0 ? "1" : "0");
    pref.putString(PREF_DEFAULT_CITY, para_default_city.getValue());
    pref.putString(PREF_WEEK_INFO, para_week_info.getValue());
    pref.putString(PREF_CD_DAY_LABLE, para_cd_day_label.getValue());
    pref.putString(PREF_CD_DAY_DATE, para_cd_day_date.getValue());
    pref.putString(PREF_TAG_DAYS, para_tag_days.getValue());
    pref.putString(PREF_SI_WEEK_1ST, strcmp(para_si_week_1st.getValue(), "1") == 0 ? "1" : "0");
    pref.putString(PREF_STUDY_SCHEDULE, para_study_schedule.getValue());
    pref.putString(PREF_WIDGETS_GOLD, strcmp(para_widgets_gold.getValue(), "1") == 0 ? "1" : "0");
    pref.end();

    Serial.println("Params saved.");

    _idle_millis = millis(); // 刷新无操作时间点

    ESP.restart();
}

void preSaveParamsCallback() {
}

// 双击打开配置页面
void buttonDoubleClick(void* oneButton) {
    Serial.println("Button double click.");
    if (wm.getConfigPortalActive()) {
        ESP.restart();
        return;
    }

    if (weather_status == 0) {
        weather_stop();
    }

    // 初始化蓝牙配置服务（仅在配置模式下启动）
    // ble_config_init();  // 暂时注释 BLE

    // 读取并显示当前电压
    int testVoltage = readBatteryVoltage();
    Serial.printf("[VOLTAGE TEST] Current battery voltage: %d mV (%.2f V)\n", 
                 testVoltage, testVoltage / 1000.0);
    
    // 显示电量状态
    if (testVoltage >= 4100) {
        Serial.println("[VOLTAGE TEST] Battery: ~90% (Very Good)");
    } else if (testVoltage >= 3900) {
        Serial.println("[VOLTAGE TEST] Battery: ~60-80% (Good)");
    } else if (testVoltage >= 3700) {
        Serial.println("[VOLTAGE TEST] Battery: ~40-50% (Medium)");
    } else if (testVoltage >= 3500) {
        Serial.println("[VOLTAGE TEST] Battery: ~10-30% (Low)");
    } else if (testVoltage >= 3000) {
        Serial.println("[VOLTAGE TEST] Battery: <10% (Very Low)");
    } else {
        Serial.println("[VOLTAGE TEST] Battery: Depleted or damaged!");
    }

    // 设置配置页面
    // 根据配置信息设置默认值
    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    String wType = pref.getString(PREF_WEATHER_TYPE, "0");
    String defaultCity = pref.getString(PREF_DEFAULT_CITY, "北京");
    String weekInfo = pref.getString(PREF_WEEK_INFO, "0");
    String cddLabel = pref.getString(PREF_CD_DAY_LABLE);
    String cddDate = pref.getString(PREF_CD_DAY_DATE);
    String tagDays = pref.getString(PREF_TAG_DAYS);
    String week1st = pref.getString(PREF_SI_WEEK_1ST, "0");
    String studySchedule = pref.getString(PREF_STUDY_SCHEDULE);
    String widgetsGold = pref.getString(PREF_WIDGETS_GOLD, "0");
    pref.end();

    para_weather_type.setValue(wType.c_str(), 1);
    para_default_city.setValue(defaultCity.c_str(), 32);
    para_week_info.setValue(weekInfo.c_str(), 2);
    para_cd_day_label.setValue(cddLabel.c_str(), 16);
    para_cd_day_date.setValue(cddDate.c_str(), 8);
    para_tag_days.setValue(tagDays.c_str(), 30);
    para_si_week_1st.setValue(week1st.c_str(), 1);
    para_study_schedule.setValue(studySchedule.c_str(), 4000);
    para_widgets_gold.setValue(widgetsGold.c_str(), 1);

    wm.setTitle("WuJinOS");
    wm.addParameter(&para_si_week_1st);
    wm.addParameter(&para_weather_type);
    wm.addParameter(&para_default_city);
    wm.addParameter(&para_week_info);
    wm.addParameter(&para_cd_day_label);
    wm.addParameter(&para_cd_day_date);
    wm.addParameter(&para_tag_days);
    wm.addParameter(&para_study_schedule);
    wm.addParameter(&para_widgets_gold);
    // std::vector<const char *> menu = {"wifi","wifinoscan","info","param","custom","close","sep","erase","update","restart","exit"};
    std::vector<const char*> menu = { "wifi","param","update","sep","info","restart","exit" };
    wm.setMenu(menu); // custom menu, pass vector
    wm.setConfigPortalBlocking(false);
    wm.setBreakAfterConfig(true);
    wm.setPreSaveParamsCallback(preSaveParamsCallback);
    wm.setSaveParamsCallback(saveParamsCallback);
    wm.setSaveConnect(false); // 保存完wifi信息后是否自动连接，设置为否，以便于用户继续配置param。
    wm.startConfigPortal("WuJinOS", "password");

    led_config(); // LED 进入三快闪状态

    // 控制配置超时180秒后休眠
    _idle_millis = millis();
}

// 重置系统，并重启
void buttonLongPressStop(void* oneButton) {
    Serial.println("Button long press.");

    // 删除Preferences，namespace下所有健值对。
    Preferences pref;
    pref.begin(PREF_NAMESPACE);
    pref.clear();
    pref.end();

    ESP.restart();
}

#define uS_TO_S_FACTOR 1000000
#define TIMEOUT_TO_SLEEP  10 // seconds
time_t blankTime = 0;
void go_sleep() {
    uint64_t p;
    // 根据天气类型决定刷新时间：每日预报模式24小时刷新，实时天气模式2小时刷新
    time_t now;
    time(&now);
    struct tm local;
    localtime_r(&now, &local);
    if (weather_type() == 0) { // 每日预报模式，第二天刷新
        // Sleep to next day
        int secondsToNextDay = (24 - local.tm_hour) * 3600 - local.tm_min * 60 - local.tm_sec;
        Serial.printf("Seconds to next day: %d seconds.\n", secondsToNextDay);
        p = (uint64_t)(secondsToNextDay);
        p = p < 0 ? 3600 * 24 : (p + 30); // 额外增加30秒，避免过早唤醒
    } else { // 实时天气模式，2小时刷新
        // Sleep to next even hour.
        int secondsToNextHour = (60 - local.tm_min) * 60 - local.tm_sec;
        if ((local.tm_hour % 2) == 0) { // 如果是奇数点，则多睡1小时
            secondsToNextHour += 3600;
        }
        Serial.printf("Seconds to next even hour: %d seconds.\n", secondsToNextHour);
        p = (uint64_t)(secondsToNextHour);
        p = p < 0 ? 3600 : (p + 10); // 额外增加10秒，避免过早唤醒
    }

    esp_sleep_enable_timer_wakeup(p * (uint64_t)uS_TO_S_FACTOR);
    esp_sleep_enable_ext0_wakeup(KEY_M, LOW);

    // 省电考虑，关闭RTC外设和存储器
    // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF); // RTC IO, sensors and ULP, 注意：由于需要按键唤醒，所以不能关闭，否则会导致RTC_IO唤醒(ext0)失败
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF); // 
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M, ESP_PD_OPTION_OFF);

    // 停止蓝牙以节省电量
    // ble_config_stop();  // 暂时注释 BLE

    gpio_deep_sleep_hold_dis(); // 解除所有引脚的保持状态
    
    // 省电考虑，重置gpio，平均每针脚能省8ua。
    // gpio_reset_pin(PIN_LED_R); // 减小deep-sleep电流
    gpio_reset_pin(SPI_CS); // 减小deep-sleep电流
    gpio_reset_pin(SPI_DC); // 减小deep-sleep电流
    gpio_reset_pin(SPI_RST); // 减小deep-sleep电流
    gpio_reset_pin(SPI_BUSY); // 减小deep-sleep电流`
    gpio_reset_pin(SPI_MOSI); // 减小deep-sleep电流
    gpio_reset_pin(SPI_MISO); // 减小deep-sleep电流
    gpio_reset_pin(SPI_SCK); // 减小deep-sleep电流
    // gpio_reset_pin(PIN_ADC); // 减小deep-sleep电流
    // gpio_reset_pin(I2C_SDA); // 减小deep-sleep电流
    // gpio_reset_pin(I2C_SCL); // 减小deep-sleep电流

    delay(10);
    Serial.println("Deep sleep...");
    Serial.flush();
    esp_deep_sleep_start();
}