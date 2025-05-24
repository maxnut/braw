#pragma once

#include "node.hpp"
#include "identifier.hpp"

#include <string>
#include <memory>

namespace AST {

struct UnaryOperatorNode : Node {
    UnaryOperatorNode() : Node(Type::UnaryOperator) {}

    std::string m_operator;
    std::shared_ptr<Node> m_data;
    std::shared_ptr<Node> m_expression = nullptr;
    std::shared_ptr<Node> m_operand;
};

}