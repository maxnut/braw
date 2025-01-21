#pragma once

#include "address.hpp"
#include "value.hpp"

typedef std::variant<Register, Value, Address, std::string> Operator;