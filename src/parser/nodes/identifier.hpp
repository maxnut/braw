#pragma once

#include "parser/nodes/node.hpp"

#include <string>

namespace AST {

struct IdentifierNode : Node {
    IdentifierNode() : Node(Type::Identifier) {}
    IdentifierNode(const std::string& name) : Node(Type::Identifier), m_name(name) {}

    std::string m_name;
};

}