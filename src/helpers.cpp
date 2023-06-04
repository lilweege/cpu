#include "helpers.hpp"

#include <fstream>

std::vector<uint8_t> ReadEntireFile(std::string_view filename)
{
    std::ifstream input(filename.data(), std::ios::binary);
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(input), {});
    return buffer;
}