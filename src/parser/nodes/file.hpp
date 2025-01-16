#pragma once

#include "function_definition.hpp"
#include "struct.hpp"
#include "bind.hpp"

#include <vector>
#include <memory>

namespace AST {

struct FileNode : Node {
    FileNode() : Node(Type::File) {}

    std::vector<std::unique_ptr<FunctionDefinitionNode>> m_functions;
    std::vector<std::unique_ptr<StructNode>> m_structs;
    std::vector<std::unique_ptr<BindNode>> m_binds;
    std::vector<std::unique_ptr<FileNode>> m_imports;
};

}