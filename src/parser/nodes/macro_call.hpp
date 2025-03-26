#pragma once

#include "parser/identifier.hpp"
#include "parser/nodes/macro_parameter.hpp"
#include "parser/nodes/node.hpp"

#include <vector>

namespace AST {

struct MacroCallNode : Node {
    MacroCallNode() : Node(Type::MacroCall) {}

    Identifier m_name;
    std::vector<std::shared_ptr<AST::MacroParameterNode>> m_parameters;
};

}