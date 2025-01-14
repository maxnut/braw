#pragma once

#include "function_definition.hpp"

namespace AST {

struct BindNode : Node {
    BindNode() : Node(Type::Bind) {}

    Identifier m_name;
    FunctionSignature m_signature;
};

}