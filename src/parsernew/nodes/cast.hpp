#pragma once

#include "node.hpp"
#include "../identifier.hpp"

#include <memory>

namespace AST {

struct CastNode : Node {
    CastNode() : Node(Type::UnaryOperator) {}

    Identifier m_type;
    std::unique_ptr<Node> m_value;
};

}