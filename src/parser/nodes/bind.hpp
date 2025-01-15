#pragma once

#include "function_definition.hpp"

namespace AST {

struct BindNode : Node {
    BindNode() : Node(Type::Bind) {}

    Identifier m_library;
    std::vector<std::pair<Identifier, FunctionSignature>> m_functions;
};

}