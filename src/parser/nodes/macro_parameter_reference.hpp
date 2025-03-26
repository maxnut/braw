
#pragma once

#include "node.hpp"
#include <string>

namespace AST {

struct MacroParameterReferenceNode : Node {
    MacroParameterReferenceNode() : Node(Type::MacroParameterReference) {}

    std::string m_name;
};

}