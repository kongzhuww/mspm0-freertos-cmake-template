# zdt_motor component CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

file(GLOB COMPONENT_SRCS "src/*.c")

# 定义 component_zdt_motor 静态库
add_library(component_zdt_motor STATIC ${COMPONENT_SRCS})

# 包含自己的头文件目录
target_include_directories(component_zdt_motor PUBLIC include)

# 链接底层的库
if(CMAKE_CROSSCOMPILING)
    target_link_libraries(component_zdt_motor PRIVATE third_party_lib)
endif()


# 假设主项目中包含 TI DriverLib 和 SysConfig 路径，链接它
# target_link_libraries(zdt_motor PRIVATE driverlib)
