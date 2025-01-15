#pragma once

#include "function_definition.hpp"
#include "struct.hpp"
#include "bind.hpp"
#include "import.hpp"

#include <vector>
#include <memory>

namespace AST {

struct FileNode : Node {
    FileNode() : Node(Type::File) {}

    std::vector<std::shared_ptr<FunctionDefinitionNode>> m_functions;
    std::vector<std::shared_ptr<StructNode>> m_structs;
    std::vector<std::shared_ptr<BindNode>> m_binds;
    std::vector<std::shared_ptr<ImportNode>> m_imports;
};

}