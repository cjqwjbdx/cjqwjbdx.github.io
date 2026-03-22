#ifndef __GOLD_H__
#define __GOLD_H__

#include <Arduino.h>

// #include "gold.h" // <--- 移除这一行

/**
 * 初始化黄金价格模块
 */
void gold_init();

/**
 * 更新黄金价格
 * @param status 指定状态，-1 表示初始化状态，其他值表示执行更新
 */
void gold_update(int8_t status = -1);

/**
 * 获取黄金价格
 * @return 黄金价格字符串
 */
String gold_get_price();

/**
 * 获取是否显示黄金价格
 * @return 是否显示黄金价格
 */
bool gold_is_enabled();

/**
 * 获取黄金价格状态
 * @return 黄金价格状态 (-1: 未初始化, 0: 待更新, 1: 更新成功, 2: 更新失败)
 */
int8_t gold_status();

/**
 * 定时检查并更新黄金价格
 * 每5秒自动刷新一次
 */
void gold_timer_check();

#endif // __GOLD_H__

