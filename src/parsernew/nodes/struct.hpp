#pragma once

#include "variable_declaration.hpp"

#include <vector>
#include <memory>

namespace AST {

struct StructNode : Node {
    StructNode() : Node(Type::Struct) {}

    Identifier m_name;
    std::vector<std::unique_ptr<VariableDeclarationNode>> m_members;
};

}