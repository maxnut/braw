#pragma once

#include "stack.hpp"
#include "memory.hpp"
#include "type_info.hpp"

#include <functional>

namespace Binder {

std::function<void(Stack&, Memory*, const std::vector<TypeInfo>&)> getFunction(const std::string& lib, const std::string& name);
void closeHandles();

}