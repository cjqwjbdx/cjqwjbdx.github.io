#include "gold.h"
#include <_preference.h>
#include "API.hpp"
#include <WiFi.h>

// 黄金价格全局变量
String _gold_price = "";
bool _show_gold = false;
int8_t _gold_status = -1;  // -1: 未初始化, 0: 待更新, 1: 更新成功, 2: 更新失败

/**
 * 初始化黄金价格模块
 * 只读取显示设置，不读取缓存数据
 */
void gold_init() {
    Preferences pref;
    pref.begin(PREF_NAMESPACE);

    // 读取是否显示黄金价格的设置
    if (pref.isKey(PREF_WIDGETS_GOLD)) {
        _show_gold = pref.getString(PREF_WIDGETS_GOLD, "0") == "1";
        Serial.printf("Gold widget enabled: %d\n", _show_gold);
    } else {
        _show_gold = false;
        Serial.println("Gold widget not found in preferences, setting to false");
    }

    // 初始化状态和价格，不读取缓存
    _gold_price = "";
    _gold_status = 0; // 标记为待更新
    
    Serial.println("Gold module initialized (cache disabled)");

    pref.end();
}

/**
 * 获取是否显示黄金价格
 * @return 是否显示黄金价格
 */
bool gold_is_enabled() {
    return _show_gold;
}

/**
 * 获取黄金价格状态
 * @return 黄金价格状态 (-1: 未初始化, 0: 待更新, 1: 更新成功, 2: 更新失败)
 */
int8_t gold_status() {
    return _gold_status;
}

/**
 * 获取黄金价格
 * @return 黄金价格字符串
 */
String gold_get_price() {
    return _gold_price;
}

/**
 * 更新黄金价格
 * @param status 指定状态，-1 表示执行更新，其他值表示设置状态
 */
void gold_update(int8_t status) {
    Serial.printf("[Gold] gold_update called, status=%d, current _gold_status=%d\n", status, _gold_status);

    // 如果指定了状态，设置状态并返回
    if (status != -1) {
        _gold_status = status;
        Serial.printf("[Gold] Setting status to %d\n", status);
        return;
    }

    // 如果已经更新成功或失败，直接返回（避免重复尝试）
    if (_gold_status == 1 || _gold_status == 2) {
        Serial.println("[Gold] Already processed (success or failure), skip");
        return;
    }

    // 如果未启用黄金价格显示，标记为失败并返回
    if (!_show_gold) {
        _gold_status = 2;
        Serial.println("[Gold] Gold widget is disabled");
        return;
    }

    // 检查网络连接状态
    if (WiFi.status() != WL_CONNECTED) {
        _gold_status = 2;
        Serial.println("[Gold] WiFi not connected, cannot fetch gold price.");
        return;
    }

    // 标记为正在处理
    _gold_status = 0;
    Serial.println("[Gold] Marked as processing");

    // 检查网络连接状态
    if (WiFi.status() != WL_CONNECTED) {
        _gold_status = 2;
        Serial.println("[Gold] WiFi not connected, cannot fetch gold price. Using cached value if any.");
        return;
    }

    Serial.println("[Gold] Attempting to fetch gold price...");

    // 尝试获取黄金价格
    API<> api;
    GoldPrice gp;

    if (api.getGoldPrice(gp)) {
        // 将"人民币/克"替换为"元/克"
        _gold_price = gp.price_text;
        _gold_price.replace("人民币/克", "元/克");
        _gold_status = 1;
        Serial.printf("[Gold] Gold price updated: %s\n", _gold_price.c_str());

        // 在串口上显示金价信息
        Serial.printf("=== 黄金价格信息 ===\n");
        Serial.printf("当前金价: %s\n", _gold_price.c_str());
        Serial.printf("===================\n");

        // 禁用缓存功能，不保存到NVS
        Serial.println("[Gold] Cache disabled - price not saved to NVS");
    } else {
        _gold_status = 2;
        Serial.println("[Gold] Failed to fetch gold price. Cache disabled - no cached value available.");
    }
}