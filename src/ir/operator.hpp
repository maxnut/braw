#pragma once

#include "address.hpp"
#include "value.hpp"
#include <variant>

typedef std::variant<std::monostate, Register, Value, Address, std::string> Operator;