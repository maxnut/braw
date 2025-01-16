#pragma once

#include "node.hpp"

#include <vector>
#include <memory>

namespace AST {

struct ScopeNode : Node {
    ScopeNode() : Node(Type::Scope) {}

    std::vector<std::unique_ptr<Node>> m_instructions;
};

}