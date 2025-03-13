#pragma once

#include <variant>
#include <string>

typedef std::variant<int, long, float, double, bool, std::string, char, std::nullptr_t> Value;