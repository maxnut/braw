#include "evaluator.hpp"
#include "braw_context.hpp"
#include "macro/node.hpp"
#include "parser/identifier.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "parser/nodes/file.hpp"
#include "parser/nodes/for.hpp"
#include "parser/nodes/function_call.hpp"
#include "parser/nodes/function_definition.hpp"
#include "parser/nodes/if.hpp"
#include "parser/nodes/literal.hpp"
#include "parser/nodes/macro.hpp"
#include "parser/nodes/macro_call.hpp"
#include "parser/nodes/macro_parameter.hpp"
#include "parser/nodes/node.hpp"
#include "parser/nodes/return.hpp"
#include "parser/nodes/scope.hpp"
#include "parser/nodes/struct.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "parser/nodes/variable_access.hpp"
#include "parser/nodes/variable_declaration.hpp"
#include "parser/nodes/while.hpp"
#include "parser/parser.hpp"
#include "spdlog/fmt/bundled/core.h"
#include <__expected/unexpected.h>
#include <memory>
#include <optional>

namespace Macro {

AST::FunctionSignature* findFunction(const std::string& name, std::shared_ptr<AST::FileNode> file) {
    AST::FunctionSignature* res;
    for(auto import : file->m_imports)
        res = findFunction(name, import);

    for(auto function : file->m_functions) {
        if(function->m_signature.m_name.m_name == name)
            return &function->m_signature;
    }

    return nullptr;
}

std::shared_ptr<Node> nodeFromType(const std::string type, BrawContext& ctx) {
    std::optional<TypeInfo> infoOpt = ctx.getTypeInfo(type);
    if(!infoOpt) return nullptr;
    std::shared_ptr<Node> ret = std::make_shared<Node>();
    ret->m_value = type;
    for(auto& member : infoOpt->m_members) {
        ret->m_members[member.first] = nodeFromType(member.second.m_type, ctx);
        if(!ret->m_members[member.first]) return nullptr;
    }
    return ret;
}

// TODO: add error messages
std::shared_ptr<Node> Evaluator::convertParameter(std::shared_ptr<AST::MacroParameterNode> param, EvaluatorContext& ctx) {
    std::shared_ptr<Node> ret = std::make_shared<Node>();
    switch(param->m_parameterType) {
        case AST::AST: {
            std::shared_ptr<AST::MacroParameterASTNode> ast = std::static_pointer_cast<AST::MacroParameterASTNode>(param);
            auto res = deepClone(ast->m_ast, ctx);
            if(!res) return nullptr;
            ret->m_node = res.value();
            break;
        }
        case AST::Type: {
            std::shared_ptr<AST::MacroParameterTypeNode> type = std::static_pointer_cast<AST::MacroParameterTypeNode>(param);
            ret = nodeFromType(type->m_type, ctx.m_ctx);
            break;
        }
        case AST::Function: {
            std::shared_ptr<AST::MacroParameterFunctionNode> function = std::static_pointer_cast<AST::MacroParameterFunctionNode>(param);
            auto func = findFunction(function->m_name, ctx.m_file);
            if(!func) return nullptr;
            ret->m_value = func->m_name;
            ret->m_members["returnType"] = nodeFromType(func->m_returnType, ctx.m_ctx);
            std::shared_ptr<Node> prms = std::make_shared<Node>();
            for(int i = 0; i < func->m_parameters.size(); i++) {
                std::shared_ptr<Node> prm = std::make_shared<Node>();
                prm->m_value = func->m_parameters[i]->m_name;
                prm->m_members["type"] = nodeFromType(func->m_parameters[i]->m_type, ctx.m_ctx);
                prms->m_members[std::to_string(i)] = prm;
            }
            ret->m_members["parameters"] = prms;
            break;
        }
        case AST::Value: {
            std::shared_ptr<AST::MacroParameterValueNode> value = std::static_pointer_cast<AST::MacroParameterValueNode>(param);
            ret->m_value = value->m_value;
            break;
        }
    }
    return ret;
}

std::shared_ptr<AST::MacroNode> findMacro(const std::string& name, std::shared_ptr<AST::FileNode> file) {
    std::shared_ptr<AST::MacroNode> res;
    for(auto import : file->m_imports)
        res = findMacro(name, import);

    if(file->m_macros.contains(name))
        return file->m_macros.at(name);

    return nullptr;
}

std::expected<std::shared_ptr<AST::Node>, MacroError> Evaluator::deepClone(std::shared_ptr<AST::Node> node, EvaluatorContext& ctx) {
    switch(node->m_type) {
        case AST::Node::File:
        case AST::Node::Macro:
        case AST::Node::FunctionDefinition:
            break;
        case AST::Node::VariableDeclaration: {
            auto decl = std::static_pointer_cast<AST::VariableDeclarationNode>(node);
            std::shared_ptr<AST::VariableDeclarationNode> clone = std::make_shared<AST::VariableDeclarationNode>(*decl);
            auto res = deepClone(decl->m_value, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_value = res.value();
            return clone;
        }
        case AST::Node::Scope: {
            auto scope = std::static_pointer_cast<AST::ScopeNode>(node);
            std::shared_ptr<AST::ScopeNode> clone = std::make_shared<AST::ScopeNode>(*scope);
            clone->m_instructions.clear(); clone->m_instructions.reserve(scope->m_instructions.size());
            for(auto& in : scope->m_instructions) {
                auto res = deepClone(in, ctx);
                if(!res) return std::unexpected{res.error()};
                clone->m_instructions.push_back(res.value());
            }
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
            for(auto& p : call->m_parameters) {
                auto res = deepClone(p, ctx);
                if(!res) return std::unexpected{res.error()};
                clone->m_parameters.push_back(res.value());
            }
            return clone;
        }
        case AST::Node::BinaryOperator: {
            auto op = std::static_pointer_cast<AST::BinaryOperatorNode>(node);
            std::shared_ptr<AST::BinaryOperatorNode> clone = std::make_shared<AST::BinaryOperatorNode>(*op);
            auto res = deepClone(op->m_left, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_left = res.value();
            res = deepClone(op->m_right, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_right = res.value();
            return clone;
        }
        case AST::Node::UnaryOperator: {
            auto op = std::static_pointer_cast<AST::UnaryOperatorNode>(node);
            std::shared_ptr<AST::UnaryOperatorNode> clone = std::make_shared<AST::UnaryOperatorNode>(*op);
            auto res = deepClone(op->m_operand, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_operand = res.value();
            res = deepClone(op->m_expression, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_expression = res.value();

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
            auto res = deepClone(ifNode->m_condition, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_condition = res.value();
            res = deepClone(ifNode->m_then, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_then = std::static_pointer_cast<AST::ScopeNode>(res.value());
            res = deepClone(ifNode->m_else, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_else = res.value();
            return clone;
        }
        case AST::Node::While: {
            auto whileNode = std::static_pointer_cast<AST::WhileNode>(node);
            std::shared_ptr<AST::WhileNode> clone = std::make_shared<AST::WhileNode>(*whileNode);
            auto res = deepClone(whileNode->m_condition, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_condition = res.value();
            res = deepClone(whileNode->m_then, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_then = std::static_pointer_cast<AST::ScopeNode>(res.value());
            return clone;
        }
        case AST::Node::For: {
            auto forNode = std::static_pointer_cast<AST::ForNode>(node);
            std::shared_ptr<AST::ForNode> clone = std::make_shared<AST::ForNode>(*forNode);
            auto res = deepClone(forNode->m_initializer, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_initializer = res.value();
            res = deepClone(forNode->m_condition, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_condition = res.value();
            res = deepClone(forNode->m_increment, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_increment = res.value();
            res = deepClone(forNode->m_body, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_body = std::static_pointer_cast<AST::ScopeNode>(res.value());
            return clone;
        }
        case AST::Node::Struct: {
            auto structNode = std::static_pointer_cast<AST::StructNode>(node);
            std::shared_ptr<AST::StructNode> clone = std::make_shared<AST::StructNode>(*structNode);
            clone->m_members.clear(); clone->m_members.reserve(structNode->m_members.size());
            for(auto& m : structNode->m_members) {
                auto res = deepClone(m, ctx);
                if(!res) return std::unexpected{res.error()};
                clone->m_members.push_back(std::static_pointer_cast<AST::VariableDeclarationNode>(res.value()));
            }
            return clone;
        }
        case AST::Node::Return: {
            auto returnNode = std::static_pointer_cast<AST::ReturnNode>(node);
            std::shared_ptr<AST::ReturnNode> clone = std::make_shared<AST::ReturnNode>(*returnNode);
            auto res = deepClone(returnNode->m_value, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_value = res.value();
            return clone;
        }
        case AST::Node::MacroParameter: {
            auto param = std::static_pointer_cast<AST::MacroParameterReferenceNode>(node);
            return ctx.m_variables.at(param->m_name)->m_node;
        }
        case AST::Node::MacroCall: {
            auto macroCallNode = std::static_pointer_cast<AST::MacroCallNode>(node);
            std::shared_ptr<AST::MacroNode> macro = findMacro(macroCallNode->m_name, ctx.m_file);
            if(!macro)
                return std::unexpected{macroNotFound(macroCallNode, ctx.m_file->m_path)};
            auto resultOrError = evaluate(macro, macroCallNode, ctx.m_file, ctx.m_ctx);
            if(!resultOrError)
                return std::unexpected{resultOrError.error()};
            return resultOrError.value();
        }
        case AST::Node::MacroParameterReference:
        case AST::Node::MacroIf:
        case AST::Node::MacroForeach:
        case AST::Node::MacroMakeFunction:
        case AST::Node::MacroMakeVariable:
          break;
        }
    return nullptr;
}

std::optional<MacroError> Evaluator::processAST(std::shared_ptr<AST::Node> node, std::shared_ptr<AST::Node>* replaceTarget, std::shared_ptr<AST::FileNode> path, BrawContext& ctx) {
    switch(node->m_type) {
        case AST::Node::Literal:
        case AST::Node::MacroParameter:
        case AST::Node::MacroParameterReference:
        case AST::Node::MacroIf:
        case AST::Node::MacroForeach:
        case AST::Node::MacroMakeFunction:
        case AST::Node::MacroMakeVariable:
        case AST::Node::Macro:
        case AST::Node::Struct:
        case AST::Node::VariableAccess:
        case AST::Node::VariableDeclaration:
            break;
        case AST::Node::FunctionDefinition: {
            auto fun = std::static_pointer_cast<AST::FunctionDefinitionNode>(node);
            processAST(fun->m_scope, (std::shared_ptr<AST::Node>*)&fun->m_scope, path, ctx);
        }
        case AST::Node::File: {
            auto file = std::static_pointer_cast<AST::FileNode>(node);
            for(auto& import : file->m_imports)
                processAST(import, (std::shared_ptr<AST::Node>*)&import, file, ctx);

            for(auto& macrocall : file->m_macroCalls)
                processAST(macrocall, (std::shared_ptr<AST::Node>*)&macrocall, file, ctx);

            for(auto& fun : file->m_functions)
                processAST(fun, (std::shared_ptr<AST::Node>*)&fun, file, ctx);
        }
        case AST::Node::Scope: {
            auto scope = std::static_pointer_cast<AST::ScopeNode>(node);
            for(auto& in : scope->m_instructions)
                processAST(in, &in, path, ctx);
            break;
        }
        case AST::Node::FunctionCall: {
            auto call = std::static_pointer_cast<AST::FunctionCallNode>(node);
            for(auto& p : call->m_parameters)
                processAST(p, &p, path, ctx);
            break;
        }
        case AST::Node::BinaryOperator: {
            auto op = std::static_pointer_cast<AST::BinaryOperatorNode>(node);
            processAST(op->m_left, &op->m_left, path, ctx);
            processAST(op->m_right, &op->m_right, path, ctx);
            break;
        }
        case AST::Node::UnaryOperator: {
            auto op = std::static_pointer_cast<AST::UnaryOperatorNode>(node);
            processAST(op->m_expression, &op->m_expression, path, ctx);
            processAST(op->m_operand, &op->m_operand, path, ctx);
            break;
        }
        case AST::Node::If: {
            auto ifNode = std::static_pointer_cast<AST::IfNode>(node);
            processAST(ifNode->m_condition, &ifNode->m_condition, path, ctx);
            processAST(ifNode->m_then, (std::shared_ptr<AST::Node>*)&ifNode->m_then, path, ctx);
            processAST(ifNode->m_else, &ifNode->m_else, path, ctx);
            break;
        }
        case AST::Node::While: {
            auto whileNode = std::static_pointer_cast<AST::WhileNode>(node);
            processAST(whileNode->m_condition, &whileNode->m_condition, path, ctx);
            processAST(whileNode->m_then, (std::shared_ptr<AST::Node>*)&whileNode->m_then, path, ctx);
            break;
        }
        case AST::Node::For: {
            auto forNode = std::static_pointer_cast<AST::ForNode>(node);
            processAST(forNode->m_initializer, &forNode->m_initializer, path, ctx);
            processAST(forNode->m_condition, &forNode->m_condition, path, ctx);
            processAST(forNode->m_increment, &forNode->m_increment, path, ctx);
            processAST(forNode->m_body, (std::shared_ptr<AST::Node>*)&forNode->m_body, path, ctx);
            break;
        }
        case AST::Node::Return: {
            auto returnNode = std::static_pointer_cast<AST::ReturnNode>(node);
            processAST(returnNode->m_value, &returnNode->m_value, path, ctx);
            break;
        }
        case AST::Node::MacroCall: {
            auto macroCallNode = std::static_pointer_cast<AST::MacroCallNode>(node);
            std::shared_ptr<AST::MacroNode> macro = findMacro(macroCallNode->m_name, path);
            if(!macro)
                return macroNotFound(macroCallNode, path->m_path);
            auto resultOrError = evaluate(macro, macroCallNode, path, ctx);
            if(!resultOrError)
                return resultOrError.error();
            *replaceTarget = resultOrError.value();
            break;
        }
    }
    return std::nullopt;
}

std::expected<std::shared_ptr<AST::Node>, MacroError> Evaluator::evaluate(std::shared_ptr<AST::MacroNode> macro, std::shared_ptr<AST::MacroCallNode> macroCall, std::shared_ptr<AST::FileNode> file, BrawContext& ctx) {
    EvaluatorContext ectx{ctx};
    ectx.m_file = file;
    for(size_t i = 0; i < macro->m_parameters.size(); i++) {
        ectx.m_variables[macro->m_parameters[i]] = convertParameter(macroCall->m_parameters[i], ectx);
    }
    return deepClone(macro->m_node, ectx);
}

std::string typeString(AST::MacroParameterType type) {
    switch(type) {
    case AST::AST:
        return "AST";
    case AST::Type:
        return "Type";
    case AST::Function:
        return "Function";
    case AST::Value:
        return "Value";
    }
}

MacroError Evaluator::unexpectedParameterTypeExpected(std::shared_ptr<AST::MacroParameterNode> causer, AST::MacroParameterType expected, const std::filesystem::path& path) {
    return MacroError {
        fmt::format("Unexpected parameter type {} expected {}", typeString(causer->m_parameterType), typeString(expected)),
        path,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    };
}

MacroError Evaluator::macroNotFound(std::shared_ptr<AST::MacroCallNode> causer, const std::filesystem::path& path) {
    return MacroError {
        fmt::format("Macro {} not found", causer->m_name.m_name),
        path,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    };
}

}