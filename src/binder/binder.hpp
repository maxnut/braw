#pragma once

#include "stack.hpp"
#include "memory.hpp"
#include "type_info.hpp"

#include <functional>
#include <filesystem>

namespace Binder {

std::function<void(Stack&, Memory*)> getFunction(const std::vector<std::filesystem::path>& libDirectories, const std::string& lib, const std::string& name);
void closeHandles();

}