#pragma once

#include "node.hpp"

#include <memory>

namespace AST {

struct VariableDeclarationNode : Node {
    VariableDeclarationNode() : Node(Type::VariableDeclaration) {}

    std::shared_ptr<Node> m_type;
    std::shared_ptr<Node> m_name;
    std::shared_ptr<Node> m_value = nullptr;
    size_t m_scale = 1;
    bool m_retain = false;
};

}