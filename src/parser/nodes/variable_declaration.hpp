#pragma once

#include "node.hpp"
#include "../identifier.hpp"

namespace AST {

struct VariableDeclarationNode : Node {
    VariableDeclarationNode() : Node(Type::VariableDeclaration) {}

    Identifier m_type;
    Identifier m_name;
};

}