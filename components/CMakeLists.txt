# 组件汇总路由 (Components Router)
# 未来每次新增一个外设组件，就在此处添加一行 add_subdirectory

# 示例：添加 dummy_sensor 组件
add_subdirectory(dummy_sensor)

# OLED SSD1306 7-Pin SPI 模拟驱动
add_subdirectory(oled)

# TB6612 双路直流电机驱动
add_subdirectory(tb6612)

# 8 路模拟灰度传感器
add_subdirectory(gray_adc)

# 小车循迹控制
add_subdirectory(control)

add_subdirectory(easy_menu)
