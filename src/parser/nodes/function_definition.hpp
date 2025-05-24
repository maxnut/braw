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
    
    std::shared_ptr<Node> m_returnType;
    std::shared_ptr<Node> m_name;
    std::vector<std::shared_ptr<AST::VariableDeclarationNode>> m_parameters; 
    bool m_external = false;
};

struct FunctionDefinitionNode : Node {
    FunctionDefinitionNode() : Node(Type::FunctionDefinition) {}

    FunctionSignature m_signature;
    std::shared_ptr<ScopeNode> m_scope;
};

}