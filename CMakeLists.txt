cmake_minimum_required(VERSION 3.10)
project(Bookstore-2025)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)
include_directories(src)

# 添加可执行文件
add_executable(code
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
