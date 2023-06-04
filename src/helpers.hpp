#pragma once

#include <cstdint>
#include <vector>
#include <string_view>

std::vector<uint8_t> ReadEntireFile(std::string_view filename);
