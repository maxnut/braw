#pragma once

#include "node.hpp"
#include "../identifier.hpp"

#include <vector>
#include <memory>

namespace AST {

struct FunctionCallNode : Node {
    FunctionCallNode() : Node(Type::FunctionCall) {}

    Identifier m_name;
    std::vector<std::unique_ptr<Node>> m_parameters;
};

}