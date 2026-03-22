# SSD1683 局刷功能实现说明

基于 CSDN 文章：https://blog.csdn.net/weixin_43550576/article/details/137375815

## 功能概述

本实现为 SSD1683 控制器（GDEY042Z98 4.2寸墨水屏）提供了优化的局刷功能，相比全刷模式：

- **刷新速度提升**：局刷 1.2秒（全刷 2.5秒）
- **功耗降低**：仅刷新局部区域
- **减少闪烁**：避免了全屏黑白闪烁
- **支持电压调节**：适配不同批次的屏幕

## 文件说明

### 核心驱动文件

1. **`lib/GxEPD2/src/gdey3c/GxEPD2_420c_GDEY042Z98_opt.h`**
   - 优化的驱动头文件
   - 添加局刷支持和电压调节接口

2. **`lib/GxEPD2/src/gdey3c/GxEPD2_420c_GDEY042Z98_opt.cpp`**
   - 优化的驱动实现
   - 包含局刷波形表（LUT）
   - 实现电压调节功能

### 示例和使用文档

3. **`examples/ssd1683_partial_update.ino`**
   - 完整的局刷使用示例
   - 展示动态内容更新
   - 电压调节示例

4. **`include/epd_opt.h`**
   - 便捷头文件，简化局刷功能调用
   - 提供常用封装函数

## 快速开始

### 1. 切换到优化驱动

在你的项目中，替换标准驱动为优化版本：

```cpp
// 原来：
// #include <GxEPD2_420c_GDEY042Z98.h>
// GxEPD2_420c_GDEY042Z98 display(SPI_CS, SPI_DC, SPI_RST, SPI_BUSY);

// 改为：
#include <gdey3c/GxEPD2_420c_GDEY042Z98_opt.h>
GxEPD2_420c_GDEY042Z98_opt display(SPI_CS, SPI_DC, SPI_RST, SPI_BUSY);
```

### 2. 初始化局刷模式

```cpp
void setup() {
  display.init(115200);
  
  // 首次使用需要全刷
  display.clearScreen(GxEPD_WHITE);
  drawStaticContent();  // 绘制静态内容
  
  // 切换到局刷模式
  display.initPartialUpdate();
}
```

### 3. 执行局部刷新

```cpp
void updateClock(int x, int y, String time) {
  // 只刷新时钟区域（假设 100x50 像素）
  display.setPartialWindow(x, y, 100, 50);
  display.firstPage();
  do {
    display.setCursor(x, y);
    display.println(time);
  } while (display.nextPage());
  
  // 或者使用简化方式：
  // display.refresh(x, y, 100, 50);
}
```

## 电压调节

不同批次的屏幕可能需要不同的电压设置：

```cpp
// 在 initPartialUpdate() 之后调用

// 批次1（默认）：
display.setGateVoltage(0x15);      // VGH = 15V
display.setSourceVoltage(0x41, 0xA8, 0x32);  // VSH1=15V, VSH2=5V, VSL=-15V
display.setVCOM(0x36);

// 批次2（显示偏淡，提高电压）：
display.setGateVoltage(0x17);      // 提高VGH
display.setSourceVoltage(0x45, 0xA8, 0x32);  // 提高VSH1
display.setVCOM(0x38);             // 提高VCOM

// 批次3（显示太深，降低电压）：
display.setGateVoltage(0x13);      // 降低VGH
display.setSourceVoltage(0x3F, 0xA8, 0x32);  // 降低VSH1
display.setVCOM(0x34);             // 降低VCOM
```

## 技术细节

### SSD1683 关键命令

根据文章，SSD1683 的电压控制命令：

- **0x03 VGH**：Gate驱动电压（10V-20V）
- **0x04 VSH1/VSH2/VSL**：Source驱动电压
  - VSH1：正电压（默认15V）
  - VSH2：中间电压（默认5V）
  - VSL：负电压（默认-15V）
- **0x2C VCOM**：VCOM电压调节

### 局刷波形表（LUT）

局刷需要特殊的波形表，本实现包含优化的227字节波形表：

```cpp
// 波形表包含：
// - 前222字节：波形数据
// - 最后5字节：电压参数
//   [0]: VGH, [1]: VSH1, [2]: VSH2, [3]: VSL, [4]: VCOM
```

### 局刷流程

1. **初始化**：调用 `initPartialUpdate()`
   - 设置扫描模式
   - 载入局刷波形表
   - 设置局刷电压参数

2. **写入数据**：使用标准 `writeImage()` 或 `setPartialWindow()`

3. **执行刷新**：调用 `refresh(x, y, w, h)`
   - 设置局部RAM区域
   - 发送局刷命令（0x22 + 0xDC）
   - 等待刷新完成（约1.2秒）

## 注意事项

1. **首次必须全刷**：墨水屏首次显示必须进行全刷，否则会有残影

2. **局刷范围限制**：局刷区域x坐标必须是8的倍数（字节边界）

3. **避免过度使用**：连续局刷多次后建议执行一次全刷，清除累积残影

4. **电压调节谨慎**：电压过高可能损坏屏幕，建议每次只调整±0x02

5. **功耗考虑**：局刷虽然省电，但频繁刷新仍会增加功耗

## 性能对比

| 刷新模式 | 时间 | 功耗 | 闪烁 | 适用场景 |
|---------|------|------|------|---------|
| 标准全刷 | 25秒 | 高 | 严重 | 首次显示、清屏 |
| 快速全刷 | 15秒 | 中 | 中等 | 定期更新 |
| **优化局刷** | **1.2秒** | **低** | **轻微** | **频繁更新** |

## 常见问题

### Q: 局刷后仍有残影？
A: 1) 检查电压设置是否合适；2) 执行一次全刷清除残影；3) 减少连续局刷次数

### Q: 局刷区域显示不完整？
A: 确保x坐标是8的倍数，宽度也按8字节对齐

### Q: 屏幕显示偏淡？
A: 尝试提高VSH1电压（增加0x02-0x04）

### Q: 如何确认局刷生效？
A: 观察刷新时屏幕是否只刷新局部区域，全刷会有明显的黑白闪烁

## 参考资料

- SSD1683 数据手册：https://v4.cecdn.yun300.cn/100001_1909185148/SSD1683.PDF
- GDEY042Z98 规格书：https://www.good-display.com/product/387.html
- CSDN 调试笔记：https://blog.csdn.net/weixin_43550576/article/details/137375815
