# ==========================================
# 小车循迹控制组件
# ==========================================

# 1. 搜集本组件下的所有源码
file(GLOB COMPONENT_SRCS "src/*.c")

# 2. 将组件打包为一个静态库
add_library(component_control STATIC ${COMPONENT_SRCS})

# 3. 暴露本组件的头文件目录
target_include_directories(component_control PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 4. 声明依赖的组件
if(CMAKE_CROSSCOMPILING)
    target_link_libraries(component_control PRIVATE component_gray_adc component_tb6612 component_oled third_party_lib)
endif()
