# 时间局刷功能实现

## 功能说明

在墨水屏日历右上角状态栏区域实现了时间局部刷新功能，每分钟自动检查并更新时间显示，仅刷新时间区域（约100x14像素），避免全屏闪烁。

## 显示位置

**位置**：屏幕右上角状态栏区域
- X坐标：`display.width() - 100` (约300px)
- Y坐标：`calLayout.statusY` (0px)
- 宽度：100px
- 高度：14px

该区域同时显示：黄金价格（左侧）、电池图标（右侧）、时间（最右侧）

## 实现文件

### 1. `include/screen_ink.h`
```cpp
// 新增函数声明
void draw_time_partial(bool partial);
void update_time_display();
```

### 2. `src/screen_ink.cpp`

**新增静态变量**：
```cpp
static char _last_displayed_time[10] = "--:--";
static bool _time_initialized = false;
```

**新增函数**：

#### `draw_time_partial(bool partial)`
- 绘制时间显示
- `partial=true`：局部刷新模式（快速）
- `partial=false`：全刷模式（首次显示）

#### `update_time_display()`
- 每分钟检查一次时间变化
- 时间变化时自动执行局刷
- 输出日志：`[Partial] Time updated: HH:MM`

**修改`task_screen()`**：
- 首次全刷时绘制时间

### 3. `src/main.cpp`

**修改`loop()`函数**：
```cpp
static uint32_t lastMinuteCheck = 0;

void loop() {
    // ... 其他代码 ...
    
    // 每分钟检查一次时间变化并执行局刷更新
    if (si_screen_status() > 0 && millis() - lastMinuteCheck > 60000) {
        lastMinuteCheck = millis();
        update_time_display();
    }
    
    // ... 休眠逻辑 ...
}
```

## 工作原理

### 时间更新流程

1. **首次显示**（`task_screen`中）：
   - 调用`draw_time_partial(false)`进行全刷
   - 保存当前时间到`_last_displayed_time`

2. **每分钟检查**（`loop`中）：
   - 检查距离上次检查是否超过60秒
   - 获取当前系统时间
   - 与`_last_displayed_time`比较

3. **时间变化时**：
   - 调用`draw_time_partial(true)`执行局刷
   - 仅刷新100x14px的区域
   - 更新`_last_displayed_time`

4. **时间未变化时**：
   - 不执行任何操作，节省功耗

### 局刷优势

- **刷新时间**：约1.2秒（vs 全刷15-25秒）
- **功耗**：极低（仅驱动局部区域）
- **用户体验**：无全屏黑白闪烁
- **更新频率**：每分钟一次，不影响主屏幕休眠周期

## 使用说明

### 启用时间局刷

功能已自动集成到主程序，无需额外配置。

### 查看日志

连接串口，波特率115200，观察日志：
```
[Partial] Time updated: 14:25
[Partial] Time updated: 14:26
[Partial] Time updated: 14:27
```

### 自定义位置（如需调整）

修改`draw_time_partial()`函数中的位置计算：
```cpp
// 原：右上角
int timeX = display.width() - timeWidth - 4;
int timeY = calLayout.statusY + calLayout.statusH - 2;

// 改为：左上角
int timeX = 4;
int timeY = 4;

// 改为：底部居中
int timeWidth = u8g2Fonts.getUTF8Width(timeStr);
int timeX = (display.width() - timeWidth) / 2;
int timeY = display.height() - 16;
```

### 调整字体大小

修改字体常量：
```cpp
// 原：小字体
u8g2Fonts.setFont(FONT_SUB);

// 改为：大字体
u8g2Fonts.setFont(u8g2_font_fub20_tn);
// 注意：需要增大刷新区域尺寸
```

## 注意事项

1. **首次必须全刷**：时间显示区域首次必须通过全刷绘制，否则会有残影

2. **屏幕休眠期间**：设备休眠时不会更新时间，唤醒后显示休眠前的时间，1分钟内会更新

3. **功耗影响**：局刷功耗极低（约全刷的1/20），对电池续航影响可忽略

4. **与其他内容共存**：时间区域与黄金价格、电池图标共享状态栏，互不干扰

5. **电压调节**：如时间显示偏淡，可参考SSD1683电压调节文档进行微调

## 调试方法

### 强制立即更新
在串口监视器中发送命令（需实现串口命令解析）：
```cpp
// 临时调用
update_time_display();
```

### 检查时间变化检测
添加调试输出：
```cpp
void update_time_display() {
    char currentTime[10];
    snprintf(currentTime, sizeof(currentTime), "%02d:%02d", tmInfo.tm_hour, tmInfo.tm_min);
    
    Serial.printf("[Debug] Current: %s, Last: %s\n", currentTime, _last_displayed_time);
    
    if (strcmp(currentTime, _last_displayed_time) != 0) {
        // ... 更新代码 ...
    }
}
```

### 验证局刷区域
在`draw_time_partial`中添加边框：
```cpp
// 绘制红色边框以验证刷新区域
display.drawRect(timeX - 2, timeY - 14, timeWidth + 4, 16, GxEPD_RED);
```

## 技术参数

- **刷新区域**：100 x 14 像素
- **刷新时间**：约1.2秒
- **检查间隔**：60秒
- **字体**：FONT_SUB（u8g2_font_unifont_t_symbols）
- **颜色**：GxEPD_BLACK（黑色）
- **位置**：屏幕右上角，右对齐

## 与其他功能的关系

| 功能 | 刷新方式 | 触发条件 | 优先级 |
|-----|---------|---------|-------|
| **时间更新** | 局刷 | 每分钟检查 | 低 |
| 日期变化 | 全刷 | 每天午夜 | 高 |
| 天气更新 | 全刷 | 每2小时 | 中 |
| 黄金价格 | 局刷 | 每小时 | 低 |
| 电池电量 | 局刷 | 每小时 | 低 |

时间局刷与其他功能互不干扰，可以同时进行。
