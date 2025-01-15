#include "ast_printer.hpp"
#include "parsernew/nodes/file.hpp"
#include "parsernew/nodes/function_definition.hpp"
#include "parsernew/nodes/scope.hpp"
#include "parsernew/nodes/variable_declaration.hpp"
#include "parsernew/nodes/variable_access.hpp"
#include "parsernew/nodes/unary_operator.hpp"
#include "parsernew/nodes/binary_operator.hpp"
#include "parsernew/nodes/struct.hpp"
#include "parsernew/nodes/function_call.hpp"
#include "parsernew/nodes/bind.hpp"
#include "parsernew/nodes/if.hpp"
#include "parsernew/nodes/while.hpp"
#include "parsernew/nodes/literal.hpp"
#include "parsernew/nodes/return.hpp"
#include "parsernew/nodes/import.hpp"

#include <spdlog/fmt/fmt.h>

#include <iostream>

std::string functionSignatureString(AST::FunctionSignature signature) {
    std::string funcString = signature.m_name.m_name + "(";
    for(int i = 0; i < signature.m_parameters.size(); i++) {
        auto& parameter = signature.m_parameters[i];
        funcString += parameter.m_type.m_name;
        if(parameter.m_name.m_name.size() > 0)
            funcString += " " + parameter.m_name.m_name;
        
        if(i < signature.m_parameters.size() - 1)
            funcString += ", ";
    }
    funcString += ")";
    return funcString;
}

std::string indentString(std::string str, int indent) {
    std::string indentStr = "";
    for(int i = 0; i < indent; i++)
        indentStr += "  ";
    return indentStr + str;
}

void ASTPrinter::print(std::shared_ptr<AST::Node> node, int indent) {
    switch(node->m_type) {
        case AST::Node::Type::File:
            print(std::static_pointer_cast<AST::FileNode>(node), indent);
            break;
        case AST::Node::Type::FunctionDefinition:
            print(std::static_pointer_cast<AST::FunctionDefinitionNode>(node), indent);
            break;
        case AST::Node::Type::Scope:
            print(std::static_pointer_cast<AST::ScopeNode>(node), indent);
            break;
        case AST::Node::Type::VariableDeclaration:
            print(std::static_pointer_cast<AST::VariableDeclarationNode>(node), indent);
            break;
        case AST::Node::Type::VariableAccess:
            print(std::static_pointer_cast<AST::VariableAccessNode>(node), indent);
            break;
        case AST::Node::Type::UnaryOperator:
            print(std::static_pointer_cast<AST::UnaryOperatorNode>(node), indent);
            break;
        case AST::Node::Type::BinaryOperator:
            print(std::static_pointer_cast<AST::BinaryOperatorNode>(node), indent);
            break;
        case AST::Node::Type::Struct:
            print(std::static_pointer_cast<AST::StructNode>(node), indent);
            break;
        case AST::Node::Type::FunctionCall:
            print(std::static_pointer_cast<AST::FunctionCallNode>(node), indent);
            break;
        case AST::Node::Type::Bind:
            print(std::static_pointer_cast<AST::BindNode>(node), indent);
            break;
        case AST::Node::Type::If:
            print(std::static_pointer_cast<AST::IfNode>(node), indent);
            break;
        case AST::Node::Type::While:
            print(std::static_pointer_cast<AST::WhileNode>(node), indent);
            break;
        case AST::Node::Type::Literal:
            print(std::static_pointer_cast<AST::LiteralNode>(node), indent);
            break;
        case AST::Node::Type::Return:
            print(std::static_pointer_cast<AST::ReturnNode>(node), indent);
            break;
        case AST::Node::Type::Import:
            print(std::static_pointer_cast<AST::ImportNode>(node), indent);
            break;
        default:
            break;
    }
}

void ASTPrinter::print(std::shared_ptr<AST::FileNode> node, int indent) {
    std::cout << "FileNode\n";

    for(auto& function : node->m_functions)
        print(function, indent + 1);

    for(auto& struct_ : node->m_structs)
        print(struct_, indent + 1);

    for(auto& bind : node->m_binds)
        print(bind, indent + 1);

    for(auto& import : node->m_imports)
        print(import, indent + 1);
}

void ASTPrinter::print(std::shared_ptr<AST::FunctionDefinitionNode> node, int indent) {
    std::cout << indentString(fmt::format("FunctionDefinitionNode {}\n", functionSignatureString(node->m_signature)), indent);
    print(node->m_scope, indent + 1);
}

void ASTPrinter::print(std::shared_ptr<AST::ScopeNode> node, int indent) {
    std::cout << indentString("ScopeNode\n", indent);
    for(auto& instruction : node->m_instructions)
        print(instruction, indent + 1);
}

void ASTPrinter::print(std::shared_ptr<AST::VariableDeclarationNode> node, int indent) {
    std::cout << indentString(fmt::format("VariableDeclarationNode {} type {}\n", node->m_name.m_name, node->m_type.m_name), indent);
}

void ASTPrinter::print(std::shared_ptr<AST::VariableAccessNode> node, int indent) {

}

void ASTPrinter::print(std::shared_ptr<AST::UnaryOperatorNode> node, int indent) {

}

void ASTPrinter::print(std::shared_ptr<AST::BinaryOperatorNode> node, int indent) {

}

void ASTPrinter::print(std::shared_ptr<AST::StructNode> node, int indent) {

}

void ASTPrinter::print(std::shared_ptr<AST::FunctionCallNode> node, int indent) {

}

void ASTPrinter::print(std::shared_ptr<AST::BindNode> node, int indent) {

}

void ASTPrinter::print(std::shared_ptr<AST::IfNode> node, int indent) {

}

void ASTPrinter::print(std::shared_ptr<AST::WhileNode> node, int indent) {

}

void ASTPrinter::print(std::shared_ptr<AST::LiteralNode> node, int indent) {

}

void ASTPrinter::print(std::shared_ptr<AST::ReturnNode> node, int indent) {

}

void ASTPrinter::print(std::shared_ptr<AST::ImportNode> node, int indent) {

}
