#pragma once

#include "identifier.hpp"
#include "parser/nodes/macro_parameter.hpp"
#include "parser/nodes/node.hpp"

#include <vector>

namespace AST {

struct MacroCallNode : Node {
    MacroCallNode() : Node(Type::MacroCall) {}

    std::shared_ptr<Node> m_name;
    std::vector<std::shared_ptr<AST::MacroParameterNode>> m_parameters;
};

}