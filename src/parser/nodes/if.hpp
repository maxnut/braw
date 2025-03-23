#pragma once

#include "scope.hpp"

namespace AST {

struct IfNode : Node {
    IfNode() : Node(Type::If) {}

    std::shared_ptr<Node> m_condition;
    std::shared_ptr<ScopeNode> m_then;
    std::shared_ptr<Node> m_else;
};

}