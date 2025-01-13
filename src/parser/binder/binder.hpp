#pragma once

#include "stack.hpp"
#include "memory.hpp"
#include "type_info.hpp"

#include <functional>

namespace Binder {

std::function<void(Stack&, Memory*)> getFunction(const std::string& lib, const std::string& name);
void closeHandles();

}