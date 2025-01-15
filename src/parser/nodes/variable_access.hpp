#pragma once

#include "node.hpp"
#include "../identifier.hpp"

namespace AST {

struct VariableAccessNode : Node {
    VariableAccessNode() : Node(Type::VariableAccess) {}

    Identifier m_name;
};

}