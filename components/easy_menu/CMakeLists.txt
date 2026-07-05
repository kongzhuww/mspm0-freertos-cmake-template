# ==========================================
# Easy_Menu 组件
# ==========================================

# 1. 搜集本组件下的所有源码
file(GLOB COMPONENT_SRCS "src/*.c")

# 2. 将组件打包为一个静态库
add_library(component_easy_menu STATIC ${COMPONENT_SRCS})

# 3. 暴露本组件的头文件目录
target_include_directories(component_easy_menu PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 4. 声明依赖的底层库
if(CMAKE_CROSSCOMPILING)
    target_link_libraries(component_easy_menu PRIVATE third_party_lib component_oled)
endif()
