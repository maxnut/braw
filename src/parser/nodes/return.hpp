#pragma once

#include "node.hpp"

#include <memory>

namespace AST {

struct ReturnNode : Node {
    ReturnNode() : Node(Type::Return) {}

    std::unique_ptr<Node> m_value = nullptr;
};

}