#pragma once

#include "identifier.hpp"
#include "parser/nodes/macro_call.hpp"
#include "parser/nodes/macro_parameter.hpp"
#include "scope.hpp"

namespace AST {

struct MacroForeachNode : Node {
    MacroForeachNode() : Node(Type::MacroForeach) {}

    std::shared_ptr<Node> m_varName;
    std::shared_ptr<MacroParameterNode> m_collection;
    std::shared_ptr<ScopeNode> m_body;
};

}