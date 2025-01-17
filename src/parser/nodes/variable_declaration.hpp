#pragma once

#include "node.hpp"
#include "../identifier.hpp"

#include <memory>

namespace AST {

struct VariableDeclarationNode : Node {
    VariableDeclarationNode() : Node(Type::VariableDeclaration) {}

    Identifier m_type;
    Identifier m_name;
    std::unique_ptr<Node> m_value = nullptr;
};

}