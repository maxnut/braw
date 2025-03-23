#pragma once

#include "scope.hpp"

namespace AST {

struct WhileNode : Node {
    WhileNode() : Node(Type::While) {}

    std::shared_ptr<Node> m_condition;
    std::shared_ptr<ScopeNode> m_then;
};

}