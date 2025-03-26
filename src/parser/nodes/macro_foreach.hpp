#pragma once

#include "parser/identifier.hpp"
#include "parser/nodes/macro_call.hpp"
#include "scope.hpp"

namespace AST {

struct MacroForeachNode : Node {
    MacroForeachNode() : Node(Type::MacroForeach) {}

    Identifier m_varName;
    std::shared_ptr<MacroCallNode> m_collection;
    std::shared_ptr<ScopeNode> m_body;
};

}