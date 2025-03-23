#pragma once

#include "scope.hpp"

namespace AST {

struct ForNode : Node {
    ForNode() : Node(Type::For) {}

    std::unique_ptr<Node> m_initializer;
    std::unique_ptr<Node> m_condition;
    std::unique_ptr<Node> m_increment;
    std::unique_ptr<ScopeNode> m_body;
};

}