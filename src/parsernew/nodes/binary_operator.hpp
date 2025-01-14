#pragma once

#include "node.hpp"

#include <string>
#include <memory>

namespace AST {

struct BinaryOperatorNode : Node {
    BinaryOperatorNode() : Node(Type::BinaryOperator) {}

    std::string m_operator;
    std::unique_ptr<Node> m_left;
    std::unique_ptr<Node> m_right;
};

}