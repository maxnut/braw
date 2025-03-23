#pragma once

#include "scope.hpp"

namespace AST {

struct ForNode : Node {
    ForNode() : Node(Type::For) {}

    std::shared_ptr<Node> m_initializer;
    std::shared_ptr<Node> m_condition;
    std::shared_ptr<Node> m_increment;
    std::shared_ptr<ScopeNode> m_body;
};

}