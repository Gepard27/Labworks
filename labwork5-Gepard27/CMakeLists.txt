cmake_minimum_required(VERSION 3.15)
project(sea_battle)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(sea_battle
    main.cpp
    src/Game.cpp
    src/CommandProcessor.cpp
)

target_include_directories(sea_battle PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)
