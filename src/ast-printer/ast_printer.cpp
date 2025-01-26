#include "ast_printer.hpp"
#include "parser/nodes/file.hpp"
#include "parser/nodes/function_definition.hpp"
#include "parser/nodes/scope.hpp"
#include "parser/nodes/variable_declaration.hpp"
#include "parser/nodes/variable_access.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "parser/nodes/struct.hpp"
#include "parser/nodes/function_call.hpp"
#include "parser/nodes/if.hpp"
#include "parser/nodes/while.hpp"
#include "parser/nodes/literal.hpp"
#include "parser/nodes/return.hpp"
#include "utils.hpp"

#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/std.h>

#include <iostream>

std::string indentString(std::string str, int indent) {
    std::string indentStr = "";
    for(int i = 0; i < indent; i++)
        indentStr += "  ";
    return indentStr + str;
}

void ASTPrinter::print(const AST::Node* node, int indent) {
    switch(node->m_type) {
        case AST::Node::Type::File:
            print(static_cast<const AST::FileNode*>(node), indent);
            break;
        case AST::Node::Type::FunctionDefinition:
            print(static_cast<const AST::FunctionDefinitionNode*>(node), indent);
            break;
        case AST::Node::Type::Scope:
            print(static_cast<const AST::ScopeNode*>(node), indent);
            break;
        case AST::Node::Type::VariableDeclaration:
            print(static_cast<const AST::VariableDeclarationNode*>(node), indent);
            break;
        case AST::Node::Type::VariableAccess:
            print(static_cast<const AST::VariableAccessNode*>(node), indent);
            break;
        case AST::Node::Type::UnaryOperator:
            print(static_cast<const AST::UnaryOperatorNode*>(node), indent);
            break;
        case AST::Node::Type::BinaryOperator:
            print(static_cast<const AST::BinaryOperatorNode*>(node), indent);
            break;
        case AST::Node::Type::Struct:
            print(static_cast<const AST::StructNode*>(node), indent);
            break;
        case AST::Node::Type::FunctionCall:
            print(static_cast<const AST::FunctionCallNode*>(node), indent);
            break;
        case AST::Node::Type::If:
            print(static_cast<const AST::IfNode*>(node), indent);
            break;
        case AST::Node::Type::While:
            print(static_cast<const AST::WhileNode*>(node), indent);
            break;
        case AST::Node::Type::Literal:
            print(static_cast<const AST::LiteralNode*>(node), indent);
            break;
        case AST::Node::Type::Return:
            print(static_cast<const AST::ReturnNode*>(node), indent);
            break;
        default:
            break;
    }
}

void ASTPrinter::print(const AST::FileNode* node, int indent) {
    std::cout << indentString("FileNode\n", indent);

    for(auto& import : node->m_imports)
        print(import.get(), indent + 1);

    for(auto& struct_ : node->m_structs)
        print(struct_.get(), indent + 1);

    for(auto& function : node->m_functions)
        print(function.get(), indent + 1);
}

void ASTPrinter::print(const AST::FunctionDefinitionNode* node, int indent) {
    std::cout << indentString(fmt::format("FunctionDefinitionNode: {}\n", Utils::functionSignatureString(node->m_signature)), indent);
    print(node->m_scope.get(), indent + 1);
}

void ASTPrinter::print(const AST::ScopeNode* node, int indent) {
    std::cout << indentString("ScopeNode\n", indent);
    for(auto& instruction : node->m_instructions)
        print(instruction.get(), indent + 1);
}

void ASTPrinter::print(const AST::VariableDeclarationNode* node, int indent) {
    std::cout << indentString(fmt::format("VariableDeclarationNode: \"{}\" ({})\n", node->m_name.m_name, node->m_type.m_name), indent);
    if(node->m_value)
        print(node->m_value.get(), indent + 1);
}

void ASTPrinter::print(const AST::VariableAccessNode* node, int indent) {
    std::cout << indentString(fmt::format("VariableAccessNode: \"{}\"\n", node->m_name.m_name), indent);
}

void ASTPrinter::print(const AST::UnaryOperatorNode* node, int indent) {
    if(node->m_data.m_name.size() > 0)
        std::cout << indentString(fmt::format("UnaryOperatorNode: {} ({})\n", node->m_operator, node->m_data.m_name), indent);
    else
        std::cout << indentString(fmt::format("UnaryOperatorNode: {}\n", node->m_operator), indent);
    
    print(node->m_operand.get(), indent + 1);
}

void ASTPrinter::print(const AST::BinaryOperatorNode* node, int indent) {
    std::cout << indentString(fmt::format("BinaryOperatorNode: {}\n", node->m_operator), indent);
    print(node->m_left.get(), indent + 1);
    print(node->m_right.get(), indent + 1);
}

void ASTPrinter::print(const AST::StructNode* node, int indent) {
    std::cout << indentString(fmt::format("StructNode: {}\n", node->m_name.m_name), indent);
    for(auto& member : node->m_members)
        print(member.get(), indent + 1);
}

void ASTPrinter::print(const AST::FunctionCallNode* node, int indent) {
    if(node->m_parameters.size() > 0) {
        std::cout << indentString(fmt::format("FunctionCallNode: {}(\n", node->m_name.m_name), indent);
        for(auto& parameter : node->m_parameters)
            print(parameter.get(), indent + 1);
        std::cout << indentString(")\n", indent);
        return;
    }

    std::cout << indentString(fmt::format("FunctionCallNode: {}()\n", node->m_name.m_name), indent);
}

void ASTPrinter::print(const AST::IfNode* node, int indent) {
    std::cout << indentString("IfNode\n", indent);
    print(node->m_condition.get(), indent + 1);
    print(node->m_then.get(), indent + 2);
    if(node->m_else)
        print(node->m_else.get(), indent + 1);
}

void ASTPrinter::print(const AST::WhileNode* node, int indent) {
    std::cout << indentString("WhileNode\n", indent);
    print(node->m_condition.get(), indent + 1);
    print(node->m_then.get(), indent + 2);
}

void ASTPrinter::print(const AST::LiteralNode* node, int indent) {
    std::cout << indentString("LiteralNode: ", indent);
    std::visit([&](auto&& arg) {
        std::cout << arg << "\n";
    }, node->m_value);
}

void ASTPrinter::print(const AST::ReturnNode* node, int indent) {
    std::cout << indentString("ReturnNode\n", indent);
    if(node->m_value)
        print(node->m_value.get(), indent + 1);
}
