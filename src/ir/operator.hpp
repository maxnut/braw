#pragma once

#include "address.hpp"
#include "value.hpp"
#include "label.hpp"

#include <variant>

typedef std::variant<std::monostate, Register, Value, Address, Label> Operator;