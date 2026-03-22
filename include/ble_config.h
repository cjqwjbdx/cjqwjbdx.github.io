#ifndef __BLE_CONFIG_H__
#define __BLE_CONFIG_H__

#include <Arduino.h>

// 蓝牙配置服务
void ble_config_init();
void ble_config_loop();
void ble_config_stop();

// 检查是否有新的配置
bool ble_has_new_config();
void ble_clear_config_flag();

// 获取配置值
String ble_get_wifi_ssid();
String ble_get_wifi_password();
String ble_get_default_city();
int ble_get_weather_type();
bool ble_get_show_gold();

#endif // __BLE_CONFIG_H__
