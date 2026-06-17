cmake_minimum_required(VERSION 3.16)

project(AsciiMediaPlayer
        VERSION 0.0.1
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXTENSIONS OFF)

find_package(OpenCV REQUIRED)

add_executable(asciimp
        src/main.cpp
        src/ascii_media_player.cpp)

target_include_directories(asciimp PRIVATE include)

target_link_libraries(asciimp PRIVATE ${OpenCV_LIBS})

set_target_properties(asciimp PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
