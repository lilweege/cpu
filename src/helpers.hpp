#pragma once

#include <cstdint>
#include <vector>
#include <string_view>

#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

struct Texture
{
    int width, height;
    GLuint id;
};

std::vector<uint8_t> ReadEntireFile(std::string_view filename);
Texture LoadTextureFromFile(const char* filename);


// Old compilers no std::bit_cast :(
template<typename T>
T bit_cast(auto value)
{
    static_assert(sizeof(T) == sizeof(value));
    T t;
    memcpy(&t, &value, sizeof(T));
    return t;
}
