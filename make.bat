@echo off
setlocal

where cl >nul 2>nul
if %ERRORLEVEL% NEQ 0 (echo ERROR: "cl" not found - please run this from the MSVC x64 native tools command prompt.) && exit /b 1
:: Either run this script from an msvc-enabled developer console, invoke the vcvars batch script
:: call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
:: call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64


set GLFW=lib/glfw-3.3.8.bin.WIN64
set IMGUI=external/imgui
set IMGUI_CLUB=external/imgui_club

set INCLUDES=/Iexternal /I%IMGUI% /I%IMGUI%/backends /I%GLFW%/include /I%IMGUI_CLUB%/imgui_memory_editor
set SOURCES=src/*.cpp %IMGUI%/backends/imgui_impl_glfw.cpp %IMGUI%/backends/imgui_impl_opengl3.cpp %IMGUI%/imgui*.cpp
set LIBS=%GLFW%/lib-vc2022/glfw3.lib opengl32.lib gdi32.lib shell32.lib

set CFLAGS=/nologo /TP /EHsc /std:c++20 /MD /W4 /wd4996 /wd4200 /Z7

@echo on
call cl %CFLAGS% /utf-8 %INCLUDES% %SOURCES% /Fecpu /link %LIBS%
