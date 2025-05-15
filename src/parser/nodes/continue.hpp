#pragma once

#include "node.hpp"

namespace AST {

struct ContinueNode : Node {
    ContinueNode() : Node(Type::Continue) {}
};

}