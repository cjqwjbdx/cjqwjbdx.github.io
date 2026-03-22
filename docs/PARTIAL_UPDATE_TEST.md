# 墨水屏局刷测试指南

## 编译错误修复 ✅

**问题已解决**：`src/main.cpp:253` 的语法错误已修复。

错误原因是多余的右大括号 `}` 导致代码结构不匹配。

## 快速开始

### 1. 启用局刷测试模式

在 `platformio.ini` 文件中，为你的环境添加编译标志：

```ini
[env:z98]
platform = espressif32
board = esp32dev
framework = arduino
build_flags = 
    -D TEST_PARTIAL_UPDATE_NO_SLEEP
    ; 其他标志...
```

### 2. 测试模式特性

启用 `TEST_PARTIAL_UPDATE_NO_SLEEP` 后：

- ✅ **不休眠**：设备不会进入深度休眠，持续运行
- ✅ **快速测试**：每10秒检查一次时间更新（而不是1分钟）
- ✅ **串口调试**：输出详细的局刷调试信息
- ✅ **快速验证**：可以立即看到局刷效果

### 3. 串口调试信息

连接串口后，你会看到：

```
[TEST] Initializing partial update mode...
[TEST] Time update check: 14:25 (partial=0)
[TEST] Partial update completed: 14:25
[TEST] Loop running... Free heap: 123456 bytes
[TEST] Time update check: 14:26 (partial=1)
[TEST] Partial update completed: 14:26
```

### 4. 局刷效果验证

1. **屏幕显示**：时间区域每10秒更新一次
2. **刷新效果**：只有右上角时间区域刷新，其他区域不变
3. **刷新时间**：约1.2秒（vs 全刷15-25秒）
4. **闪烁程度**：轻微闪烁（vs 全刷严重闪烁）

## 正常模式使用

### 禁用测试模式

注释掉 `platformio.ini` 中的测试标志：

```ini
build_flags = 
    ; -D TEST_PARTIAL_UPDATE_NO_SLEEP
    ; 其他标志...
```

### 正常模式特性

- 设备在刷新完成后自动休眠
- 每分钟检查一次时间更新
- 时间变化时执行局刷
- 极低的功耗

## 技术细节

### 局刷实现原理

1. **局部窗口**：只刷新时间显示区域（100x14像素）
2. **局刷初始化**：调用 `display.initPartialUpdate()`
3. **快速刷新**：使用优化的电压设置和波形
4. **低功耗**：局刷后调用 `display.powerOff()`

### 时间更新流程

```
main.loop()
  └─> 每60秒检查
      └─> update_time_display()
          └─> 比较当前时间 vs 上次显示时间
              └─> 时间变化 → draw_time_partial(true)
                  └─> 局刷时间区域
                      └─> display.setPartialWindow()
                      └─> display.firstPage()
                      └─> 绘制新时间
                      └─> display.nextPage()
                      └─> display.powerOff()
```

## 常见问题

### Q: 局刷后其他区域变黑了？
A: 检查是否在 `draw_cal_days()` 中设置了 `u8g2Fonts.setBackgroundColor(GxEPD_WHITE)`

### Q: 局刷没有效果？
A: 确保使用的是优化驱动 `GxEPD2_420c_GDEY042Z98_opt`

### Q: 如何调整局刷区域？
A: 修改 `draw_time_partial()` 中的 `timeX`, `timeY`, `timeW`, `timeH` 参数

### Q: 局刷有残影？
A: 调整电压参数：`setGateVoltage()`, `setSourceVoltage()`, `setVCOM()`

## 性能对比

| 模式 | 刷新时间 | 功耗 | 闪烁程度 |
|------|---------|------|---------|
| 全刷 | 15-25秒 | 高 | 严重 |
| 局刷 | 1.2秒 | 极低 | 轻微 |

## 注意事项

1. 测试模式仅用于开发调试，正常使用请禁用
2. 局刷频率不宜过高，建议至少30秒一次
3. 不同批次的屏幕可能需要微调电压参数
4. 长时间不休眠可能导致墨水屏损坏

## 代码位置

- 局刷函数：`src/screen_ink.cpp` → `draw_time_partial()`
- 更新检查：`src/screen_ink.cpp` → `update_time_display()`
- 测试宏：`src/screen_ink.cpp` 和 `src/main.cpp`
- 优化驱动：`lib/GxEPD2/src/gdey3c/GxEPD2_420c_GDEY042Z98_opt.*`
