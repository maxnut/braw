#pragma once

#include <variant>
#include <string>

typedef std::variant<int, long, float, double, bool, std::string, char, uint32_t, uint64_t> Value;