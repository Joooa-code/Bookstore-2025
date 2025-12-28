cmake_minimum_required(VERSION 3.10)
project(Bookstore-2025)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置可执行文件名
set(EXECUTABLE_OUTPUT_NAME code)

# 添加可执行文件
add_executable(${EXECUTABLE_OUTPUT_NAME}
        src/Book.cpp
        src/main.cpp
        include/Account.h
        include/Book.h
        include/Storage.h
        include/Log.h
        src/Log.cpp
        src/Storage.cpp
        src/Account.cpp
        include/MemoryRiver.h
)

# 包含目录
target_include_directories(${EXECUTABLE_OUTPUT_NAME} PRIVATE include)