#pragma once

#include "scope.hpp"

namespace AST {

struct WhileNode : Node {
    WhileNode() : Node(Type::While) {}

    std::unique_ptr<Node> m_condition;
    std::unique_ptr<ScopeNode> m_then;
};

}