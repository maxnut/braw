#pragma once

#include "variable_declaration.hpp"
#include "scope.hpp"

#include <vector>
#include <memory>

namespace AST {

struct FunctionSignature {
    Identifier m_returnType;
    Identifier m_name;
    std::vector<VariableDeclarationNode> m_parameters; 
};

struct FunctionDefinitionNode : Node {
    FunctionDefinitionNode() : Node(Type::FunctionDefinition) {}

    FunctionSignature m_signature;
    std::shared_ptr<ScopeNode> m_scope;
};

}