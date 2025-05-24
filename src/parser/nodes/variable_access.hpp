#pragma once

#include "node.hpp"
#include "identifier.hpp"
#include <memory>

namespace AST {

struct VariableAccessNode : Node {
    VariableAccessNode() : Node(Type::VariableAccess) {}

    std::shared_ptr<Node> m_name;
};

}