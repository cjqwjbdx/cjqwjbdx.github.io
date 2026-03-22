#include "ble_config.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include "_preference.h"

// BLE UUIDs - 与小程序匹配
#define SERVICE_UUID        "0000FFE0-0000-1000-8000-00805F9B34FB"
#define CHARACTERISTIC_UUID "0000FFE1-0000-1000-8000-00805F9B34FB"

// 设备名称
#define DEVICE_NAME "WuJinOS"

// 全局变量
static BLEServer *pServer = NULL;
static BLECharacteristic *pCharacteristic = NULL;
static bool deviceConnected = false;
static bool hasNewConfig = false;

// 配置数据
static String new_ssid = "";
static String new_password = "";
static String new_city = "";
static int new_weather_type = -1;
static bool new_show_gold = false;

// 函数声明
void handle_ble_command(const char* data);
void handle_scan_wifi();
void handle_set_wifi(JsonObject data);
void handle_set_params(JsonObject data);
void handle_reset_params();
void send_response(const char* cmd, const char* data);
void send_response_raw(const char* data);

// BLE 回调类
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("[BLE] 设备已连接");
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("[BLE] 设备已断开");
        // 重新启动广播
        BLEAdvertising *pAdvertising = pServer->getAdvertising();
        pAdvertising->start();
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            Serial.print("[BLE] 收到数据: ");
            Serial.println(value.c_str());
            handle_ble_command(value.c_str());
        }
    }
};

// 处理蓝牙命令
void handle_ble_command(const char* data) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);

    if (error) {
        Serial.print("[BLE] JSON解析失败: ");
        Serial.println(error.c_str());
        send_response("error", "{\"success\":false,\"msg\":\"invalid json\"}");
        return;
    }

    const char* cmd = doc["cmd"];
    if (!cmd) {
        send_response("error", "{\"success\":false,\"msg\":\"no cmd\"}");
        return;
    }

    Serial.printf("[BLE] 执行命令: %s\n", cmd);

    if (strcmp(cmd, "scan_wifi") == 0) {
        handle_scan_wifi();
    }
    else if (strcmp(cmd, "set_wifi") == 0) {
        handle_set_wifi(doc["data"]);
    }
    else if (strcmp(cmd, "set_params") == 0) {
        handle_set_params(doc["data"]);
    }
    else if (strcmp(cmd, "reset_params") == 0) {
        handle_reset_params();
    }
    else {
        send_response("error", "{\"success\":false,\"msg\":\"unknown cmd\"}");
    }
}

// 扫描WiFi
void handle_scan_wifi() {
    Serial.println("[BLE] 开始扫描WiFi...");
    int n = WiFi.scanNetworks();

    JsonDocument doc;
    doc["cmd"] = "wifi_scan_result";
    JsonObject data = doc["data"].to<JsonObject>();
    JsonArray list = data["list"].to<JsonArray>();

    for (int i = 0; i < n && i < 10; i++) {
        JsonObject wifi = list.add<JsonObject>();
        wifi["ssid"] = WiFi.SSID(i);
        wifi["rssi"] = WiFi.RSSI(i);
        wifi["auth"] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
    }

    String response;
    serializeJson(doc, response);
    send_response_raw(response.c_str());
}

// 设置WiFi
void handle_set_wifi(JsonObject data) {
    if (!data["ssid"] || !data["password"]) {
        send_response("set_wifi_result", "{\"success\":false,\"msg\":\"missing params\"}");
        return;
    }

    new_ssid = data["ssid"].as<String>();
    new_password = data["password"].as<String>();

    Serial.printf("[BLE] 设置WiFi: %s\n", new_ssid.c_str());

    // WiFiManager 会自动保存，这里不需要额外保存
    // 只需要重启让 WiFiManager 使用新配置
    hasNewConfig = true;
    send_response("set_wifi_result", "{\"success\":true}");
}

// 设置参数
void handle_set_params(JsonObject data) {
    Preferences pref;
    pref.begin(PREF_NAMESPACE);

    if (data["default_city"]) {
        new_city = data["default_city"].as<String>();
        pref.putString(PREF_DEFAULT_CITY, new_city);
    }

    if (data["api_key"]) {
        String new_api_key = data["api_key"].as<String>();
        pref.putString(PREF_API_KEY, new_api_key);
        Serial.printf("[BLE] API密钥已保存: %s\n", new_api_key.c_str());
    }

    if (data["weather_type"]) {
        new_weather_type = data["weather_type"];
        pref.putString(PREF_WEATHER_TYPE, new_weather_type > 0 ? "1" : "0");
    }

    if (data["widgets_gold"]) {
        new_show_gold = strcmp(data["widgets_gold"], "on") == 0;
        pref.putString(PREF_WIDGETS_GOLD, new_show_gold ? "1" : "0");
    }

    // 其他参数...
    if (data["week_info"]) {
        pref.putString(PREF_WEEK_INFO, data["week_info"].as<String>());
    }
    if (data["tag_days"]) {
        pref.putInt(PREF_TAG_DAYS, data["tag_days"]);
    }
    if (data["cd_day_label"]) {
        pref.putString(PREF_CD_DAY_LABLE, data["cd_day_label"].as<String>());
    }
    if (data["cd_day_date"]) {
        pref.putString(PREF_CD_DAY_DATE, data["cd_day_date"].as<String>());
    }

    pref.end();
    hasNewConfig = true;
    send_response("set_params_result", "{\"success\":true}");
    Serial.println("[BLE] 参数已保存");
}

// 重置参数
void handle_reset_params() {
    Preferences pref;
    pref.begin(PREF_NAMESPACE);

    // 恢复默认配置
    pref.putString(PREF_DEFAULT_CITY, "北京");
    pref.putString(PREF_WEATHER_TYPE, "1");
    pref.putString(PREF_WIDGETS_GOLD, "1");
    pref.putString(PREF_WEEK_INFO, "0");
    pref.putInt(PREF_TAG_DAYS, 7);
    pref.putString(PREF_CD_DAY_LABLE, "");
    pref.putString(PREF_CD_DAY_DATE, "");

    pref.end();
    send_response("reset_result", "{\"success\":true}");
    Serial.println("[BLE] 参数已重置");
}

// 发送响应
void send_response(const char* cmd, const char* data) {
    JsonDocument doc;
    doc["cmd"] = cmd;

    // 解析 data JSON 字符串并赋值
    JsonDocument dataDoc;
    DeserializationError error = deserializeJson(dataDoc, data);
    if (error) {
        doc["data"] = data;
    } else {
        doc["data"] = dataDoc;
    }

    String response;
    serializeJson(doc, response);
    send_response_raw(response.c_str());
}

void send_response_raw(const char* data) {
    if (deviceConnected && pCharacteristic) {
        pCharacteristic->setValue(data);
        pCharacteristic->notify();
        Serial.printf("[BLE] 发送响应: %s\n", data);
    }
}

// 初始化BLE
void ble_config_init() {
    Serial.println("[BLE] 初始化蓝牙...");

    // 创建BLE设备
    BLEDevice::init(DEVICE_NAME);

    // 创建BLE服务器
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // 创建BLE服务
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // 创建BLE特征
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->addDescriptor(new BLE2902());

    // 启动服务
    pService->start();

    // 启动广播
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    Serial.println("[BLE] 蓝牙已启动，等待连接...");
}

// BLE 循环处理
void ble_config_loop() {
    // 可以在这里添加定期发送状态等逻辑
}

// 停止BLE
void ble_config_stop() {
    if (pServer) {
        pServer->getAdvertising()->stop();
    }
    BLEDevice::deinit();
    Serial.println("[BLE] 蓝牙已停止");
}

// 检查是否有新配置
bool ble_has_new_config() {
    return hasNewConfig;
}

// 清除配置标志
void ble_clear_config_flag() {
    hasNewConfig = false;
}

// 获取配置值
String ble_get_wifi_ssid() { return new_ssid; }
String ble_get_wifi_password() { return new_password; }
String ble_get_default_city() { return new_city; }
int ble_get_weather_type() { return new_weather_type; }
bool ble_get_show_gold() { return new_show_gold; }
