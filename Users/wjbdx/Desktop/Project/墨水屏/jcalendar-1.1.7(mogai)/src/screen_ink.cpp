// 在农历信息显示部分，调整位置使其靠近日字
int lunar_x = x + 80; // 调整x坐标，使农历信息更靠近日字
int lunar_y = y + 60; // 保持y坐标不变

// 显示农历年份 (如"二〇二四")
display->drawString(lunar_x, lunar_y, lunar_year);

// 计算月份字符串宽度，以实现精确间隔
int month_width = display->getStringWidth(lunar_month);
display->drawString(lunar_x + display->getStringWidth(lunar_year) + 15, lunar_y, lunar_month);

// 计算日字符串宽度，以实现精确间隔
int day_width = display->getStringWidth(lunar_day);
display->drawString(lunar_x + display->getStringWidth(lunar_year) + display->getStringWidth(lunar_month) + 30, lunar_y, lunar_day);