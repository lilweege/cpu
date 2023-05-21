:: reset lib dir
if exist lib rmdir /s /q lib
mkdir lib

:: download and extract libs
curl --output-dir lib -o glfw.zip -fsSLO https://github.com/glfw/glfw/releases/download/3.3.8/glfw-3.3.8.bin.WIN64.zip
tar -x -f lib\glfw.zip -C lib
del lib\glfw.zip
