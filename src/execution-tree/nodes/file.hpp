#pragma once

#include "type_info.hpp"
#include "function_definition.hpp"
#include "node.hpp"

#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

class FileNode : public Node {
public:
    FileNode() : Node(Type::File) {}

    std::vector<std::shared_ptr<FunctionDefinitionNode>> m_functions;
    std::vector<std::shared_ptr<FileNode>> m_imports;
};