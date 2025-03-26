#pragma once

#include "parser/nodes/macro_call.hpp"
#include "scope.hpp"

namespace AST {

struct MacroIfNode : Node {
    MacroIfNode() : Node(Type::MacroIf) {}

    std::shared_ptr<MacroCallNode> m_condition;
    std::shared_ptr<ScopeNode> m_then;
};

}