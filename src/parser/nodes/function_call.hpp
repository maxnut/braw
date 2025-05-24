#pragma once

#include "node.hpp"
#include "identifier.hpp"

#include <vector>
#include <memory>

namespace AST {

struct FunctionCallNode : Node {
    FunctionCallNode() : Node(Type::FunctionCall) {}

    std::shared_ptr<Node> m_name;
    std::vector<std::shared_ptr<Node>> m_parameters;
};

}