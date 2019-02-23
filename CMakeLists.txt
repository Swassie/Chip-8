cmake_minimum_required(VERSION 3.9)
project(CHIP-8)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

set(SOURCES
  src/Display.cpp
  src/Display.h
  src/InitIO.cpp
  src/InitIO.h
  src/Keyboard.cpp
  src/Keyboard.h
  src/main.cpp
  src/Sound.cpp
  src/Sound.h
  src/VM.cpp
  src/VM.h
  )

add_executable(CHIP-8 ${SOURCES})
find_package(SDL2 REQUIRED)
target_link_libraries(CHIP-8 ${SDL2_LIBRARIES})
