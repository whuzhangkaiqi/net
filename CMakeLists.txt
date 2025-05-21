cmake_minimum_required(VERSION 3.5)
project(mymuduo)

# 编译成so库，设置动态库的输出路径，放在根目录的lib目录下
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置调试信息
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")

# 定义参与编译的源文件，把根目录下所有文件名以.cpp结尾的文件都加入编译列表
aux_source_directory(. SRC_LIST)

# 编译生成动态库
add_library(mymuduo SHARED ${SRC_LIST})
