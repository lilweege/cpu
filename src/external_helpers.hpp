#pragma once

#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

struct Texture
{
    int width, height;
    GLuint id;
};

Texture LoadTextureFromFile(const char* filename);

