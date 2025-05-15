
#pragma once

#include "node.hpp"

namespace AST {

struct BreakNode : Node {
    BreakNode() : Node(Type::Break) {}
};

}