#pragma once

#include "node.hpp"
#include "identifier.hpp"
#include <memory>
#include <string>

namespace AST {

enum MacroParameterType {
    AST,
    Type,
    Function,
    Value,
    Dot
};

struct MacroParameterNode : Node {
    MacroParameterNode() : Node(Type::MacroParameter) {}

    MacroParameterType m_parameterType;
};

struct MacroParameterASTNode : MacroParameterNode {
    MacroParameterASTNode() : MacroParameterNode() { m_parameterType = MacroParameterType::AST; }
    std::shared_ptr<AST::Node> m_ast;
};

struct MacroParameterTypeNode : MacroParameterNode {
    MacroParameterTypeNode() : MacroParameterNode() { m_parameterType = MacroParameterType::Type; }
    std::string m_type;
};

struct MacroParameterFunctionNode : MacroParameterNode {
    MacroParameterFunctionNode() : MacroParameterNode() { m_parameterType = MacroParameterType::Function; }
    std::string m_name;
};

struct MacroParameterValueNode : MacroParameterNode {
    MacroParameterValueNode() : MacroParameterNode() { m_parameterType = MacroParameterType::Value; }
    std::string m_value;
};

struct MacroParameterDotNode : MacroParameterNode {
    MacroParameterDotNode() : MacroParameterNode() { m_parameterType = MacroParameterType::Value; }
    std::string m_value;
    std::shared_ptr<AST::MacroParameterNode> m_prev;
};

}