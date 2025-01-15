#pragma once

#include "scope.hpp"

namespace AST {

struct IfNode : Node {
    IfNode() : Node(Type::If) {}

    std::unique_ptr<Node> m_condition;
    std::unique_ptr<ScopeNode> m_then;
    std::unique_ptr<Node> m_else;
};

}