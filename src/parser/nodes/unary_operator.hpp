#pragma once

#include "node.hpp"
#include "../identifier.hpp"

#include <string>
#include <memory>

namespace AST {

struct UnaryOperatorNode : Node {
    UnaryOperatorNode() : Node(Type::UnaryOperator) {}

    std::string m_operator;
    Identifier m_data;
    std::unique_ptr<Node> m_operand;
};

}