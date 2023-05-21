#!/usr/bin/env bash

# Install glfw from package manager
# sudo apt install libglfw3
IMGUI="external/imgui"

INCLUDES="-I$IMGUI -I$IMGUI/backends"
SOURCES="src/*.cpp $IMGUI/backends/imgui_impl_glfw.cpp $IMGUI/backends/imgui_impl_opengl3.cpp $IMGUI/imgui*.cpp"
LIBS="-lglfw -lrt -lm -ldl -lGL"

CFLAGS="-std=c++20 -g -Wall -Wextra -Wshadow -Wswitch-enum -Wpedantic -Werror -O2"

set -xe

g++ $CFLAGS $INCLUDES $SOURCES $LIBS -o cpu.out
