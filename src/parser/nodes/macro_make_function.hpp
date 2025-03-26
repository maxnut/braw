#pragma once

#include "parser/nodes/macro_parameter.hpp"
#include "scope.hpp"

namespace AST {

struct MacroMakeFunctionNode : Node {
    MacroMakeFunctionNode() : Node(Type::MacroMakeFunction) {}

    std::shared_ptr<MacroParameterNode> m_name;
    std::shared_ptr<MacroParameterNode> m_returnType;
    std::shared_ptr<ScopeNode> m_parameterContainer;
    std::shared_ptr<ScopeNode> m_body;
};

}