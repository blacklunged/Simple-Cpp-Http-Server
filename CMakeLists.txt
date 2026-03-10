cmake_minimum_required(VERSION 3.10)
project(HttpServer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(http_server
    src/main.cpp
    src/Config.cpp 
    src/Socket.cpp
    src/HttpRequest.cpp
    src/HttpResponse.cpp
    src/HttpClientHandler.cpp
    src/Logger.cpp
    src/ThreadPool.cpp
    src/Server.cpp
)

target_include_directories(http_server PRIVATE include)

find_package(Threads REQUIRED)
target_link_libraries(http_server Threads::Threads)

target_compile_options(http_server PRIVATE -Wall -Wextra -Wpedantic)

