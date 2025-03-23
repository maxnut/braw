#pragma once

#include "node.hpp"
#include "../identifier.hpp"

#include <cstdint>
#include <memory>

namespace AST {

struct VariableDeclarationNode : Node {
    VariableDeclarationNode() : Node(Type::VariableDeclaration) {}

    Identifier m_type;
    Identifier m_name;
    std::shared_ptr<Node> m_value = nullptr;
    size_t m_scale = 1;
};

}