#pragma once

#include "node.hpp"

#include <cstdint>
#include <variant>
#include <string>

namespace AST {

struct LiteralNode : Node {
    LiteralNode() : Node(Type::Literal) {}

    std::variant<int, long, float, double, bool, std::string, char, uint32_t, uint64_t> m_value;
};

}