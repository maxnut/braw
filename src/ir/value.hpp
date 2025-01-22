#pragma once

#include <variant>
#include <string>

typedef std::variant<int, long, float, double, bool, std::string, std::nullptr_t> Value;