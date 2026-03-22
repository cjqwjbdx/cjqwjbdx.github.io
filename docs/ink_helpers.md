# ink_helpers 使用速查表 ✅

目的：集中项目中常用的打印/绘制逻辑，统一对 u8g2 的调用习惯，减少重复代码。

示例用法：

```cpp
// 设置前景色并在坐标处绘制
ink_begin(GxEPD_BLACK);
ink_print(10, 20, "Hello", u8g2_font_fub14_tn, GxEPD_BLACK, GxEPD_WHITE);

// 打印并格式化（在当前 cursor 输出）
ink_printfc("%s %d", "Day", 6);

// 使用高级帮助器绘制日历单元格
ink_draw_day_cell(0, 80, 6, GxEPD_BLACK, "初六", "\u2605", "");
```

说明：更多详细示例请参见 `src/ink_helpers.cpp` 中函数实现上的注释。
