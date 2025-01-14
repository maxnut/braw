#pragma once

#include "node.hpp"

#include <string>
#include <memory>

namespace AST {

struct UnaryOperatorNode : Node {
    UnaryOperatorNode() : Node(Type::UnaryOperator) {}

    std::string m_operator;
    std::unique_ptr<Node> m_operand;
};

}