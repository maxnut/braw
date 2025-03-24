#include "parser/identifier.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "parser/nodes/for.hpp"
#include "parser/nodes/function_call.hpp"
#include "parser/nodes/if.hpp"
#include "parser/nodes/literal.hpp"
#include "parser/nodes/macro_parameter.hpp"
#include "parser/nodes/return.hpp"
#include "parser/nodes/scope.hpp"
#include "parser/nodes/struct.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "parser/nodes/variable_access.hpp"
#include "parser/nodes/variable_declaration.hpp"
#include "parser/nodes/while.hpp"
#include "parser/parser.hpp"
#include <memory>

std::shared_ptr<AST::Node> deepClone(std::shared_ptr<AST::Node> node, const std::vector<std::shared_ptr<AST::Node>>& parameters) {
    switch(node->m_type) {
        case AST::Node::File:
        case AST::Node::Macro:
        case AST::Node::FunctionDefinition:
            break;
        case AST::Node::VariableDeclaration: {
            auto decl = std::static_pointer_cast<AST::VariableDeclarationNode>(node);
            std::shared_ptr<AST::VariableDeclarationNode> clone = std::make_shared<AST::VariableDeclarationNode>(*decl);
            clone->m_value = deepClone(decl->m_value, parameters);
        }
        case AST::Node::Scope: {
            auto scope = std::static_pointer_cast<AST::ScopeNode>(node);
            std::shared_ptr<AST::ScopeNode> clone = std::make_shared<AST::ScopeNode>(*scope);
            clone->m_instructions.clear(); clone->m_instructions.reserve(scope->m_instructions.size());
            for(auto& in : scope->m_instructions)
                clone->m_instructions.push_back(deepClone(in, parameters));
            return clone;
        }
        case AST::Node::VariableAccess: {
            auto access = std::static_pointer_cast<AST::VariableAccessNode>(node);
            std::shared_ptr<AST::VariableAccessNode> clone = std::make_shared<AST::VariableAccessNode>(*access);
            return clone;
        }
        case AST::Node::FunctionCall: {
            auto call = std::static_pointer_cast<AST::FunctionCallNode>(node);
            std::shared_ptr<AST::FunctionCallNode> clone = std::make_shared<AST::FunctionCallNode>(*call);
            clone->m_parameters.clear(); clone->m_parameters.reserve(call->m_parameters.size());
            for(auto& p : call->m_parameters)
                clone->m_parameters.push_back(deepClone(p, parameters));
            return clone;
        }
        case AST::Node::BinaryOperator: {
            auto op = std::static_pointer_cast<AST::BinaryOperatorNode>(node);
            std::shared_ptr<AST::BinaryOperatorNode> clone = std::make_shared<AST::BinaryOperatorNode>(*op);
            clone->m_left = deepClone(op->m_left, parameters);
            clone->m_right = deepClone(op->m_right, parameters);
            return clone;
        }
        case AST::Node::UnaryOperator: {
            auto op = std::static_pointer_cast<AST::UnaryOperatorNode>(node);
            std::shared_ptr<AST::UnaryOperatorNode> clone = std::make_shared<AST::UnaryOperatorNode>(*op);
            clone->m_expression = deepClone(op->m_expression, parameters);
            clone->m_operand = deepClone(op->m_operand, parameters);
            return clone;
        }
        case AST::Node::Literal: {
            auto lit = std::static_pointer_cast<AST::LiteralNode>(node);
            std::shared_ptr<AST::LiteralNode> clone = std::make_shared<AST::LiteralNode>(*lit);
            return clone;
        }
        case AST::Node::If: {
            auto ifNode = std::static_pointer_cast<AST::IfNode>(node);
            std::shared_ptr<AST::IfNode> clone = std::make_shared<AST::IfNode>(*ifNode);
            clone->m_condition = deepClone(ifNode->m_condition, parameters);
            clone->m_then = std::static_pointer_cast<AST::ScopeNode>(deepClone(ifNode->m_then, parameters));
            clone->m_else = deepClone(ifNode->m_else, parameters);
            return clone;
        }
        case AST::Node::While: {
            auto whileNode = std::static_pointer_cast<AST::WhileNode>(node);
            std::shared_ptr<AST::WhileNode> clone = std::make_shared<AST::WhileNode>(*whileNode);
            clone->m_condition = deepClone(whileNode->m_condition, parameters);
            clone->m_then = std::static_pointer_cast<AST::ScopeNode>(deepClone(whileNode->m_then, parameters));
            return clone;
        }
        case AST::Node::For: {
            auto forNode = std::static_pointer_cast<AST::ForNode>(node);
            std::shared_ptr<AST::ForNode> clone = std::make_shared<AST::ForNode>(*forNode);
            clone->m_initializer = deepClone(forNode->m_initializer, parameters);
            clone->m_condition = deepClone(forNode->m_condition, parameters);
            clone->m_increment = deepClone(forNode->m_increment, parameters);
            clone->m_body = std::static_pointer_cast<AST::ScopeNode>(deepClone(forNode->m_body, parameters));
            return clone;
        }
        case AST::Node::Struct: {
            auto structNode = std::static_pointer_cast<AST::StructNode>(node);
            std::shared_ptr<AST::StructNode> clone = std::make_shared<AST::StructNode>(*structNode);
            clone->m_members.clear(); clone->m_members.reserve(structNode->m_members.size());
            for(auto& m : structNode->m_members)
                clone->m_members.push_back(std::static_pointer_cast<AST::VariableDeclarationNode>(deepClone(m, parameters)));
            return clone;
        }
        case AST::Node::Return: {
            auto returnNode = std::static_pointer_cast<AST::ReturnNode>(node);
            std::shared_ptr<AST::ReturnNode> clone = std::make_shared<AST::ReturnNode>(*returnNode);
            clone->m_value = deepClone(returnNode->m_value, parameters);
            return clone;
        }
        case AST::Node::MacroParameter:
            auto param = std::static_pointer_cast<AST::MacroParameterNode>(node);
            return parameters.at(param->m_index);
    }
    return nullptr;
}

Result<std::shared_ptr<AST::Node>> Parser::parseMacroCall(TokenCursor& cursor, ParserContext& ctx) {
    Identifier name = cursor.next().get().next().value().m_value;
    if(!ctx.m_file->m_macros.contains(name))
        return unknownMacro(cursor.value(), ctx.m_path);

    std::vector<std::shared_ptr<AST::Node>> parameters;

    if(cursor.get().value().m_type == Token::LEFT_PAREN) {
        cursor.tryNext();
        while(cursor.hasNext()) {
            auto pOpt = parseInstruction(cursor, ctx);
            if(!pOpt)
                return std::unexpected{pOpt.error()};
            parameters.push_back(std::move(pOpt.value()));
            if(cursor.get().value().m_type == Token::RIGHT_PAREN)
                break;
            if(!expectTokenType(cursor.get().next().value(), Token::COMMA))
                return unexpectedTokenExpectedType(cursor.value(), Token::COMMA, ctx.m_path);
        }
        cursor.tryNext();
    }
    return deepClone(ctx.m_file->m_macros[name]->m_node, parameters);
}