#pragma once

#include "node.hpp"

#include <variant>
#include <string>

namespace AST {

struct LiteralNode : Node {
    LiteralNode() : Node(Type::Literal) {}

    std::variant<int, long, float, double, bool, std::string> m_value;
};

}