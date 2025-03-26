#pragma once

#include "parser/nodes/macro_parameter.hpp"
#include "scope.hpp"

namespace AST {

struct MacroMakeVariableNode : Node {
    MacroMakeVariableNode() : Node(Type::MacroMakeVariable) {}

    std::shared_ptr<MacroParameterNode> m_name;
    std::shared_ptr<MacroParameterNode> m_type;
};

}