#pragma once

#include "address.hpp"
#include "value.hpp"
#include "label.hpp"

#include <memory>
#include <variant>

typedef std::variant<std::monostate, std::shared_ptr<Register>, Value, Address, Label> Operand;