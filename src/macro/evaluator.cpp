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
#include "parser/nodes/macro_foreach.hpp"
#include "parser/nodes/macro_if.hpp"
#include "parser/nodes/macro_make_function.hpp"
#include "parser/nodes/macro_make_variable.hpp"
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
#include <__expected/unexpect.h>
#include <__expected/unexpected.h>
#include <memory>
#include <optional>
#include <unordered_map>

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

std::expected<std::shared_ptr<Node>, MacroError> Evaluator::nodeFromType(const std::string type, std::shared_ptr<AST::Node> node, EvaluatorContext& ctx) {
    std::optional<TypeInfo> infoOpt = ctx.m_ctx.getTypeInfo(type);
    if(!infoOpt) return std::unexpected{unknownType(node, type, ctx.m_file->m_path)};
    std::shared_ptr<Node> ret = std::make_shared<Node>();
    ret->m_value = type;
    for(auto& member : infoOpt->m_members) {
        auto res = nodeFromType(member.second.m_type, node, ctx);
        if(!res) return std::unexpected{res.error()};
        ret->m_members[member.first] = res.value();
        ret->m_members[member.first]->m_members["name"] = std::make_shared<Node>(member.first);
    }
    return ret;
}

std::expected<std::shared_ptr<Node>, MacroError> Evaluator::convertParameter(std::shared_ptr<AST::MacroParameterNode> param, EvaluatorContext& ctx) {
    std::shared_ptr<Node> ret = std::make_shared<Node>();
    switch(param->m_parameterType) {
        case AST::AST: {
            std::shared_ptr<AST::MacroParameterASTNode> ast = std::static_pointer_cast<AST::MacroParameterASTNode>(param);
            auto res = deepClone(ast->m_ast, ctx);
            if(!res) return std::unexpected{res.error()};
            return res.value();
        }
        case AST::Type: {
            std::shared_ptr<AST::MacroParameterTypeNode> type = std::static_pointer_cast<AST::MacroParameterTypeNode>(param);
            auto typeOpt = nodeFromType(type->m_type, type, ctx);
            if(!typeOpt) return std::unexpected{typeOpt.error()};
            ret = typeOpt.value();
            break;
        }
        case AST::Function: {
            std::shared_ptr<AST::MacroParameterFunctionNode> function = std::static_pointer_cast<AST::MacroParameterFunctionNode>(param);
            auto func = findFunction(function->m_name, ctx.m_file);
            if(!func) return std::unexpected{unknownFunction(function, ctx.m_file->m_path)};
            ret->m_value = func->m_name;
            auto typeOpt = nodeFromType(func->m_returnType, function, ctx);
            if(!typeOpt) return std::unexpected{typeOpt.error()};
            ret->m_members["returnType"] = typeOpt.value();
            std::shared_ptr<Node> prms = std::make_shared<Node>();
            for(int i = 0; i < func->m_parameters.size(); i++) {
                std::shared_ptr<Node> prm = std::make_shared<Node>();
                prm->m_value = func->m_parameters[i]->m_name;
                typeOpt = nodeFromType(func->m_parameters[i]->m_type, function, ctx);
                if(!typeOpt) return std::unexpected{typeOpt.error()};
                prm->m_members["type"] = typeOpt.value();
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
        case AST::Dot: {
            std::shared_ptr<AST::MacroParameterDotNode> dot = std::static_pointer_cast<AST::MacroParameterDotNode>(param);
            auto innerConvertedOpt = convertParameter(dot->m_prev, ctx);
            if(!innerConvertedOpt) return std::unexpected{innerConvertedOpt.error()};
            std::shared_ptr<Node> innerConverted = innerConvertedOpt.value();
            if(!innerConverted || !innerConverted->m_members.contains(dot->m_value)) return nullptr;
            return innerConverted->m_members.at(dot->m_value);
        }
        break;
    }
    return ret;
}

std::shared_ptr<AST::MacroNode> findMacro(const std::string& name, std::shared_ptr<AST::FileNode> file) {
    static std::unordered_map<std::string, std::shared_ptr<AST::MacroNode>> builtin = {
        {"compare", std::shared_ptr<AST::MacroNode>(new AST::MacroNode("compare", nullptr, {"left", "right"}))},
        {"not", std::shared_ptr<AST::MacroNode>(new AST::MacroNode("not", nullptr, {"value"}))},
        {"and", std::shared_ptr<AST::MacroNode>(new AST::MacroNode("and", nullptr, {"left", "right"}))},
        {"or", std::shared_ptr<AST::MacroNode>(new AST::MacroNode("or", nullptr, {"left", "right"}))},
        {"make_dot", std::shared_ptr<AST::MacroNode>(new AST::MacroNode("make_dot", nullptr, {"left", "right"}))},
        {"make_arrow", std::shared_ptr<AST::MacroNode>(new AST::MacroNode("make_arrow", nullptr, {"left", "right"}))},
        {"concat", std::shared_ptr<AST::MacroNode>(new AST::MacroNode("concat", nullptr, {"left", "right"}))}
    };

    if(builtin.contains(name))
        return builtin.at(name);
    
    std::shared_ptr<AST::MacroNode> res;
    for(auto import : file->m_imports)
        res = findMacro(name, import);

    if(file->m_macros.contains(name))
        return file->m_macros.at(name);

    return nullptr;
}

std::expected<std::shared_ptr<Node>, MacroError> Evaluator::deepClone(std::shared_ptr<AST::Node> node, EvaluatorContext& ctx) {
    std::shared_ptr<Node> ret = std::make_shared<Node>();
    switch(node->m_type) {
        case AST::Node::VariableDeclaration: {
            auto decl = std::static_pointer_cast<AST::VariableDeclarationNode>(node);
            std::shared_ptr<AST::VariableDeclarationNode> clone = std::make_shared<AST::VariableDeclarationNode>(*decl);
            if(clone->m_value) {
                auto res = deepClone(decl->m_value, ctx);
                if(!res) return std::unexpected{res.error()};
                clone->m_value = res.value()->m_node;
            }
            ret->m_node = clone;
            break;
        }
        case AST::Node::Scope: {
            auto scope = std::static_pointer_cast<AST::ScopeNode>(node);
            std::shared_ptr<AST::ScopeNode> clone = std::make_shared<AST::ScopeNode>(*scope);
            clone->m_instructions.clear(); clone->m_instructions.reserve(scope->m_instructions.size());
            for(auto& in : scope->m_instructions) {
                auto res = deepClone(in, ctx);
                if(!res) return std::unexpected{res.error()};
                if(res.value()->m_node)
                    clone->m_instructions.push_back(res.value()->m_node);
            }
            ret->m_node = clone;
            break;
        }
        case AST::Node::VariableAccess: {
            auto access = std::static_pointer_cast<AST::VariableAccessNode>(node);
            std::shared_ptr<AST::VariableAccessNode> clone = std::make_shared<AST::VariableAccessNode>(*access);
            ret->m_node = clone;
            break;
        }
        case AST::Node::FunctionCall: {
            auto call = std::static_pointer_cast<AST::FunctionCallNode>(node);
            std::shared_ptr<AST::FunctionCallNode> clone = std::make_shared<AST::FunctionCallNode>(*call);
            clone->m_parameters.clear(); clone->m_parameters.reserve(call->m_parameters.size());
            for(auto& p : call->m_parameters) {
                auto res = deepClone(p, ctx);
                if(!res) return std::unexpected{res.error()};
                clone->m_parameters.push_back(res.value()->m_node);
            }
            ret->m_node = clone;
            break;
        }
        case AST::Node::BinaryOperator: {
            auto op = std::static_pointer_cast<AST::BinaryOperatorNode>(node);
            std::shared_ptr<AST::BinaryOperatorNode> clone = std::make_shared<AST::BinaryOperatorNode>(*op);
            auto res = deepClone(op->m_left, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_left = res.value()->m_node;
            res = deepClone(op->m_right, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_right = res.value()->m_node;
            ret->m_node = clone;
            break;
        }
        case AST::Node::UnaryOperator: {
            auto op = std::static_pointer_cast<AST::UnaryOperatorNode>(node);
            std::shared_ptr<AST::UnaryOperatorNode> clone = std::make_shared<AST::UnaryOperatorNode>(*op);
            auto res = deepClone(op->m_operand, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_operand = res.value()->m_node;
            if(op->m_expression) {
                res = deepClone(op->m_expression, ctx);
                if(!res) return std::unexpected{res.error()};
                clone->m_expression = res.value()->m_node;
            }
            ret->m_node = clone;
            break;
        }
        case AST::Node::Literal: {
            auto lit = std::static_pointer_cast<AST::LiteralNode>(node);
            std::shared_ptr<AST::LiteralNode> clone = std::make_shared<AST::LiteralNode>(*lit);
            ret->m_node = clone;
            break;
        }
        case AST::Node::If: {
            auto ifNode = std::static_pointer_cast<AST::IfNode>(node);
            std::shared_ptr<AST::IfNode> clone = std::make_shared<AST::IfNode>(*ifNode);
            auto res = deepClone(ifNode->m_condition, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_condition = res.value()->m_node;
            res = deepClone(ifNode->m_then, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_then = std::static_pointer_cast<AST::ScopeNode>(res.value()->m_node);
            if(ifNode->m_else) {
                res = deepClone(ifNode->m_else, ctx);
                if(!res) return std::unexpected{res.error()};
                clone->m_else = res.value()->m_node;
            }
            ret->m_node = clone;
            break;
        }
        case AST::Node::While: {
            auto whileNode = std::static_pointer_cast<AST::WhileNode>(node);
            std::shared_ptr<AST::WhileNode> clone = std::make_shared<AST::WhileNode>(*whileNode);
            auto res = deepClone(whileNode->m_condition, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_condition = res.value()->m_node;
            res = deepClone(whileNode->m_then, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_then = std::static_pointer_cast<AST::ScopeNode>(res.value()->m_node);
            ret->m_node = clone;
            break;
        }
        case AST::Node::For: {
            auto forNode = std::static_pointer_cast<AST::ForNode>(node);
            std::shared_ptr<AST::ForNode> clone = std::make_shared<AST::ForNode>(*forNode);
            auto res = deepClone(forNode->m_initializer, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_initializer = res.value()->m_node;
            res = deepClone(forNode->m_condition, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_condition = res.value()->m_node;
            res = deepClone(forNode->m_increment, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_increment = res.value()->m_node;
            res = deepClone(forNode->m_body, ctx);
            if(!res) return std::unexpected{res.error()};
            clone->m_body = std::static_pointer_cast<AST::ScopeNode>(res.value()->m_node);
            ret->m_node = clone;
            break;
        }
        case AST::Node::Struct: {
            auto structNode = std::static_pointer_cast<AST::StructNode>(node);
            std::shared_ptr<AST::StructNode> clone = std::make_shared<AST::StructNode>(*structNode);
            clone->m_members.clear(); clone->m_members.reserve(structNode->m_members.size());
            for(auto& m : structNode->m_members) {
                auto res = deepClone(m, ctx);
                if(!res) return std::unexpected{res.error()};
                clone->m_members.push_back(std::static_pointer_cast<AST::VariableDeclarationNode>(res.value()->m_node));
            }
            ret->m_node = clone;
            break;
        }
        case AST::Node::Return: {
            auto returnNode = std::static_pointer_cast<AST::ReturnNode>(node);
            std::shared_ptr<AST::ReturnNode> clone = std::make_shared<AST::ReturnNode>(*returnNode);
            if(returnNode->m_value) {
                auto res = deepClone(returnNode->m_value, ctx);
                if(!res) return std::unexpected{res.error()};
                clone->m_value = res.value()->m_node;
            }
            ret->m_node = clone;
            break;
        }
        case AST::Node::MacroParameterReference: {
            auto param = std::static_pointer_cast<AST::MacroParameterReferenceNode>(node);
            return ctx.m_variables.at(param->m_name);
            break;
        }
        case AST::Node::MacroCall: {
            auto macroCallNode = std::static_pointer_cast<AST::MacroCallNode>(node);
            std::shared_ptr<AST::MacroNode> macro = findMacro(macroCallNode->m_name, ctx.m_file);
            if(!macro)
                return std::unexpected{unknownMacro(macroCallNode, ctx.m_file->m_path)};
            auto resultOrError = evaluate(macro, macroCallNode, ctx.m_file, ctx);
            if(!resultOrError)
                return std::unexpected{resultOrError.error()};
            return resultOrError.value();
        }
        case AST::Node::MacroIf: {
            auto macroIfNode = std::static_pointer_cast<AST::MacroIfNode>(node);
            auto expRes = evaluate(findMacro("compare", ctx.m_file), macroIfNode->m_condition, ctx.m_file, ctx);
            if(!expRes) return std::unexpected{expRes.error()};
            if(expRes.value()->m_value == "true") {
                auto clone = deepClone(macroIfNode->m_then, ctx);
                if(!clone) return std::unexpected{clone.error()};
                ret->m_node = clone.value()->m_node;
            }
            break;
        }
        case AST::Node::MacroForeach: {
            auto macroForeachNode = std::static_pointer_cast<AST::MacroForeachNode>(node);
            auto collectionOpt = convertParameter(macroForeachNode->m_collection, ctx);
            if(!collectionOpt) return std::unexpected{collectionOpt.error()};
            auto collection = collectionOpt.value();
            std::shared_ptr<AST::ScopeNode> scope = std::make_shared<AST::ScopeNode>();
            for(auto& pair : collection->m_members) {
                ctx.m_variables[macroForeachNode->m_varName] = pair.second;
                auto clone = deepClone(macroForeachNode->m_body, ctx);
                if(!clone) return std::unexpected{clone.error()};
                auto cloneBody = std::static_pointer_cast<AST::ScopeNode>(clone.value()->m_node);
                scope->m_instructions.insert(scope->m_instructions.end(), cloneBody->m_instructions.begin(), cloneBody->m_instructions.end());
            }
            ret->m_node = scope;
            break;
        }
        case AST::Node::MacroMakeFunction: {
            auto macroMakeFunctionNode = std::static_pointer_cast<AST::MacroMakeFunctionNode>(node);
            auto nameOpt = convertParameter(macroMakeFunctionNode->m_name, ctx);
            if(!nameOpt) return std::unexpected{nameOpt.error()};
            auto name = nameOpt.value();
            auto typeOpt = convertParameter(macroMakeFunctionNode->m_returnType, ctx);
            if(!typeOpt) return std::unexpected{nameOpt.error()};
            auto type = typeOpt.value();

            std::shared_ptr<AST::FunctionDefinitionNode> def = std::make_shared<AST::FunctionDefinitionNode>();
            def->m_rangeBegin = node->m_rangeBegin;
            def->m_rangeEnd = node->m_rangeEnd;
            def->m_signature.m_name = name->m_value;
            def->m_signature.m_returnType = type->m_value;
            
            for(auto instr : macroMakeFunctionNode->m_parameterContainer->m_instructions) {
                auto instrClone = deepClone(instr, ctx);
                if(!instrClone) return std::unexpected{instrClone.error()};
                if(instrClone.value()->m_node->m_type != AST::Node::VariableDeclaration)
                    return std::unexpected{notVariableDeclaration(instrClone.value()->m_node, ctx.m_file->m_path)};
                def->m_signature.m_parameters.push_back(std::static_pointer_cast<AST::VariableDeclarationNode>(instrClone.value()->m_node));
            }
            auto bodyOpt = deepClone(macroMakeFunctionNode->m_body, ctx);
            if(!bodyOpt) return std::unexpected{bodyOpt.error()};
            def->m_scope = std::static_pointer_cast<AST::ScopeNode>(bodyOpt.value()->m_node);
            ret->m_node = def;
            break;
        }
        case AST::Node::MacroMakeVariable: {
            auto macroMakeVariableNode = std::static_pointer_cast<AST::MacroMakeVariableNode>(node);
            auto nameOpt = convertParameter(macroMakeVariableNode->m_name, ctx);
            if(!nameOpt) return std::unexpected{nameOpt.error()};
            auto name = nameOpt.value();
            auto typeOpt = convertParameter(macroMakeVariableNode->m_type, ctx);
            if(!typeOpt) return std::unexpected{typeOpt.error()};
            auto type = typeOpt.value();
            std::shared_ptr<AST::VariableDeclarationNode> decl = std::make_shared<AST::VariableDeclarationNode>();
            decl->m_rangeBegin = macroMakeVariableNode->m_rangeBegin;
            decl->m_rangeEnd = macroMakeVariableNode->m_rangeEnd;
            decl->m_name = name->m_value;
            decl->m_type = type->m_value;
            ret->m_node = decl;
            break;
        }
        case AST::Node::MacroParameter:
        case AST::Node::MacroDot:
        case AST::Node::File:
        case AST::Node::Macro:
        case AST::Node::FunctionDefinition:
            break;
        }
    return ret;
}

std::vector<std::shared_ptr<AST::Node>> extractInstruction(std::shared_ptr<AST::ScopeNode> node) {
    std::vector<std::shared_ptr<AST::Node>> res;

    for(auto& instr : node->m_instructions) {
        if(instr->m_type == AST::Node::Scope) {
            auto scope = std::static_pointer_cast<AST::ScopeNode>(instr);
            auto instrs = extractInstruction(scope);
            res.insert(res.end(), instrs.begin(), instrs.end());
            continue;
        }
        res.push_back(instr);
    }

    return res;
}

std::optional<MacroError> Evaluator::processAST(std::shared_ptr<AST::Node> node, std::shared_ptr<AST::Node>* replaceTarget, std::shared_ptr<AST::FileNode> path, BrawContext& ctx) {
    switch(node->m_type) {
        case AST::Node::Literal:
        case AST::Node::MacroDot:
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
            if(!fun->m_signature.m_external)
                processAST(fun->m_scope, (std::shared_ptr<AST::Node>*)&fun->m_scope, path, ctx);
            break;
        }
        case AST::Node::File: {
            auto file = std::static_pointer_cast<AST::FileNode>(node);
            for(auto& import : file->m_imports)
                processAST(import, (std::shared_ptr<AST::Node>*)&import, file, ctx);

            for(auto& fun : file->m_functions)
                processAST(fun, (std::shared_ptr<AST::Node>*)&fun, file, ctx);

            for(auto& make : file->m_macroCalls) {
                std::shared_ptr<AST::Node> tmp;
                auto err = processAST(make, &tmp, file, ctx);
                if(err) return err;
                if(tmp->m_type == AST::Node::FunctionDefinition)
                    file->m_functions.insert(file->m_functions.begin(), std::static_pointer_cast<AST::FunctionDefinitionNode>(tmp));
                else if(tmp->m_type == AST::Node::Scope) {
                    auto instrs = extractInstruction(std::static_pointer_cast<AST::ScopeNode>(tmp));
                    for(auto& fun : instrs) {
                        if(fun->m_type != AST::Node::FunctionDefinition) return notFunctionDefinition(fun, file->m_path);
                        file->m_functions.insert(file->m_functions.begin(), std::static_pointer_cast<AST::FunctionDefinitionNode>(fun));
                    }
                }
                else
                    return notFunctionDefinition(tmp, file->m_path);
            }
            break;
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
            if(op->m_expression)
                processAST(op->m_expression, &op->m_expression, path, ctx);
            processAST(op->m_operand, &op->m_operand, path, ctx);
            break;
        }
        case AST::Node::If: {
            auto ifNode = std::static_pointer_cast<AST::IfNode>(node);
            processAST(ifNode->m_condition, &ifNode->m_condition, path, ctx);
            processAST(ifNode->m_then, (std::shared_ptr<AST::Node>*)&ifNode->m_then, path, ctx);
            if(ifNode->m_else)
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
            if(returnNode->m_value)
                processAST(returnNode->m_value, &returnNode->m_value, path, ctx);
            break;
        }
        case AST::Node::MacroCall: {
            auto macroCallNode = std::static_pointer_cast<AST::MacroCallNode>(node);
            std::shared_ptr<AST::MacroNode> macro = findMacro(macroCallNode->m_name, path);
            if(!macro)
                return unknownMacro(macroCallNode, path->m_path);
            EvaluatorContext tmpCtx{ctx,path};
            auto resultOrError = evaluate(macro, macroCallNode, path, tmpCtx);
            if(!resultOrError)
                return resultOrError.error();
            *replaceTarget = resultOrError.value()->m_node;
            break;
        }
    }
    return std::nullopt;
}

std::shared_ptr<Node> macroCompare(EvaluatorContext& ctx) {
    std::shared_ptr<Node> ret = std::make_shared<Node>();
    ret->m_value = ctx.m_variables["left"]->m_value == ctx.m_variables["right"]->m_value ? "true" : "false";
    return ret;
}

std::shared_ptr<Node> macroNot(EvaluatorContext& ctx) {
    std::shared_ptr<Node> ret = std::make_shared<Node>();
    std::string val = ctx.m_variables["value"]->m_value;
    ret->m_value = val == "true" ? "false" : val == "false" ? "true" : "";
    return ret;
}

std::shared_ptr<Node> macroAnd(EvaluatorContext& ctx) {
    std::shared_ptr<Node> ret = std::make_shared<Node>();
    ret->m_value = ctx.m_variables["left"]->m_value == "true" && ctx.m_variables["right"]->m_value == "true" ? "true" : "false";
    return ret;
}

std::shared_ptr<Node> macroOr(EvaluatorContext& ctx) {
    std::shared_ptr<Node> ret = std::make_shared<Node>();
    ret->m_value = ctx.m_variables["left"]->m_value == "true" || ctx.m_variables["right"]->m_value == "true" ? "true" : "false";
    return ret;
}

std::shared_ptr<Node> macroConcat(EvaluatorContext& ctx) {
    std::shared_ptr<Node> ret = std::make_shared<Node>();
    ret->m_value = ctx.m_variables["left"]->m_value + ctx.m_variables["right"]->m_value;
    return ret;
}

std::shared_ptr<Node> macroMakeDot(EvaluatorContext& ctx, std::shared_ptr<AST::MacroCallNode> call) {
    std::shared_ptr<Node> ret = std::make_shared<Node>();
    std::shared_ptr<AST::UnaryOperatorNode> dot = std::make_shared<AST::UnaryOperatorNode>();
    dot->m_rangeBegin = call->m_rangeBegin;
    dot->m_rangeEnd = call->m_rangeEnd;
    dot->m_operator = ".";
    dot->m_operand = ctx.m_variables["left"]->m_node;
    dot->m_data = ctx.m_variables["right"]->m_value;
    ret->m_node = dot;
    return ret;
}

std::shared_ptr<Node> macroMakeArrow(EvaluatorContext& ctx, std::shared_ptr<AST::MacroCallNode> call) {
    std::shared_ptr<Node> ret = std::make_shared<Node>();
    std::shared_ptr<AST::UnaryOperatorNode> dot = std::make_shared<AST::UnaryOperatorNode>();
    dot->m_rangeBegin = call->m_rangeBegin;
    dot->m_rangeEnd = call->m_rangeEnd;
    dot->m_operator = "->";
    dot->m_operand = ctx.m_variables["left"]->m_node;
    dot->m_data = ctx.m_variables["right"]->m_value;
    ret->m_node = dot;
    return ret;
}

std::expected<std::shared_ptr<Node>, MacroError> Evaluator::evaluate(std::shared_ptr<AST::MacroNode> macro, std::shared_ptr<AST::MacroCallNode> macroCall, std::shared_ptr<AST::FileNode> file, EvaluatorContext& old) {
    EvaluatorContext ectx{old.m_ctx};
    ectx.m_file = file;
    if(macro->m_parameters.size() != macroCall->m_parameters.size())
        return std::unexpected{expectedParamCount(macroCall, macroCall->m_parameters.size(), macro->m_parameters.size(), file->m_path)};
    for(size_t i = 0; i < macro->m_parameters.size(); i++) {
        auto paramOpt = convertParameter(macroCall->m_parameters[i], old);
        if(!paramOpt) return std::unexpected{paramOpt.error()};
        ectx.m_variables[macro->m_parameters[i]] = paramOpt.value();
    }

    if(macro->m_name == "compare")
        return macroCompare(ectx);
    else if(macro->m_name == "concat")
        return macroConcat(ectx);
    else if(macro->m_name == "not")
        return macroNot(ectx);
    else if(macro->m_name == "and")
        return macroAnd(ectx);
    else if(macro->m_name == "or")
        return macroOr(ectx);
    else if(macro->m_name == "make_dot")
        return macroMakeDot(ectx, macroCall);
    else if(macro->m_name == "make_arrow")
        return macroMakeArrow(ectx, macroCall);
    
    auto nodeOr = deepClone(macro->m_node, ectx);
    if(!nodeOr) return std::unexpected{nodeOr.error()};

    std::shared_ptr<AST::ScopeNode> scope = std::static_pointer_cast<AST::ScopeNode>(nodeOr.value()->m_node);
    if(scope->m_instructions.size() == 1 && scope->m_instructions.at(0)->m_type != AST::Node::Scope)
        nodeOr.value()->m_node = scope->m_instructions.at(0);
    
    return nodeOr.value();
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
    case AST::Dot:
        return "Dot";
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

MacroError Evaluator::unknownMacro(std::shared_ptr<AST::MacroCallNode> causer, const std::filesystem::path& path) {
    return MacroError {
        fmt::format("Unknown macro {}", causer->m_name.m_name),
        path,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    };
}

MacroError Evaluator::unknownFunction(std::shared_ptr<AST::MacroParameterFunctionNode> causer, const std::filesystem::path& path) {
    return MacroError {
        fmt::format("Unknown function {}", causer->m_name.m_name),
        path,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    };
}

MacroError Evaluator::unknownType(std::shared_ptr<AST::Node> causer, const std::string& type, const std::filesystem::path& path) {
    return MacroError {
        fmt::format("Unknown type {}", type),
        path,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    };
}

MacroError Evaluator::notVariableDeclaration(std::shared_ptr<AST::Node> causer, const std::filesystem::path& path) {
    return MacroError {
        fmt::format("Not a variable declaration"),
        path,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    };
}

MacroError Evaluator::notFunctionDefinition(std::shared_ptr<AST::Node> causer, const std::filesystem::path& path) {
    return MacroError {
        fmt::format("Not a function definition"),
        path,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    };
}

MacroError Evaluator::expectedParamCount(std::shared_ptr<AST::Node> causer, int got, int expected, const std::filesystem::path& path) {
    return MacroError {
        fmt::format("Expected {} parameters, got {}", expected, got),
        path,
        causer->m_rangeBegin,
        causer->m_rangeEnd
    };
}

}