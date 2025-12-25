cmake_minimum_required(VERSION 3.10)
project(Bookstore-2025)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILE
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

add_executable(Bookstore-2025 ${SOURCE_FILE})


target_include_directories(Bookstore-2025 PRIVATE
        include
)
include_directories(include)