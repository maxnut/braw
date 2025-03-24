
#pragma once

#include "node.hpp"

#include <cstdint>

namespace AST {

struct MacroParameterNode : Node {
    MacroParameterNode() : Node(Type::MacroParameter) {}

    uint32_t m_index = 0;
};

}