#pragma once

#include "type_info.hpp"
#include "function_definition.hpp"

#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

class FileNode {
public:
    std::unordered_map<std::string, std::vector<std::shared_ptr<FunctionDefinitionNode>>> m_functionTable;
};