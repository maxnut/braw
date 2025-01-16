#pragma once

#include "scope.hpp"
#include "variable_declaration.hpp"

namespace AST {

struct FunctionSignature {
    FunctionSignature() = default;
    FunctionSignature(const FunctionSignature&) = delete;
    FunctionSignature& operator=(const FunctionSignature&) = delete;

    FunctionSignature(FunctionSignature&&) = default;
    FunctionSignature& operator=(FunctionSignature&&) = default;
    
    Identifier m_returnType;
    Identifier m_name;
    std::vector<std::unique_ptr<AST::VariableDeclarationNode>> m_parameters; 
};

struct FunctionDefinitionNode : Node {
    FunctionDefinitionNode() : Node(Type::FunctionDefinition) {}

    FunctionSignature m_signature;
    std::unique_ptr<ScopeNode> m_scope;
};

}