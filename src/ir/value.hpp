#pragma once

#include <variant>
#include <string>

typedef std::variant<int, long, float, double, std::string> Value;