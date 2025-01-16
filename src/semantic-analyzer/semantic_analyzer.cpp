#include "semantic_analyzer.hpp"
#include "parser/nodes/file.hpp"
#include "parser/nodes/function_definition.hpp"
#include "parser/nodes/scope.hpp"
#include "parser/nodes/variable_declaration.hpp"
#include "parser/nodes/variable_access.hpp"
#include "parser/nodes/unary_operator.hpp"
#include "parser/nodes/binary_operator.hpp"
#include "parser/nodes/struct.hpp"
#include "parser/nodes/function_call.hpp"
#include "parser/nodes/bind.hpp"
#include "parser/nodes/if.hpp"
#include "parser/nodes/while.hpp"
#include "parser/nodes/literal.hpp"
#include "parser/nodes/return.hpp"
#include "utils.hpp"

#include <spdlog/fmt/fmt.h>

std::expected<BrawContext, SemanticError> SemanticAnalyzer::analyze(const AST::FileNode* file) {
    BrawContext ctx;
    std::optional<SemanticError> errorOpt = analyze(file, ctx);

    if(errorOpt)
        return std::unexpected{errorOpt.value()};
    return ctx;
}

std::optional<SemanticError> SemanticAnalyzer::analyze(const AST::Node* root, BrawContext& context) {
    switch (root->m_type) {
        case AST::Node::File:
            return analyze(static_cast<const AST::FileNode*>(root), context);
        case AST::Node::FunctionDefinition:
            return analyze(static_cast<const AST::FunctionDefinitionNode*>(root), context);
        case AST::Node::Scope:
            return analyze(static_cast<const AST::ScopeNode*>(root), context);
        case AST::Node::VariableDeclaration:
            return analyze(static_cast<const AST::VariableDeclarationNode*>(root), context);
        case AST::Node::VariableAccess:
            return analyze(static_cast<const AST::VariableAccessNode*>(root), context);
        case AST::Node::UnaryOperator:
            return analyze(static_cast<const AST::UnaryOperatorNode*>(root), context);
        case AST::Node::BinaryOperator:
            return analyze(static_cast<const AST::BinaryOperatorNode*>(root), context);
        case AST::Node::Struct:
            return analyze(static_cast<const AST::StructNode*>(root), context);
        case AST::Node::FunctionCall:
            return analyze(static_cast<const AST::FunctionCallNode*>(root), context);
        case AST::Node::Bind:
            return analyze(static_cast<const AST::BindNode*>(root), context);
        case AST::Node::If:
            return analyze(static_cast<const AST::IfNode*>(root), context);
        case AST::Node::While:
            return analyze(static_cast<const AST::WhileNode*>(root), context);
        case AST::Node::Literal:
            return analyze(static_cast<const AST::LiteralNode*>(root), context);
        case AST::Node::Return:
            return analyze(static_cast<const AST::ReturnNode*>(root), context);
    }
    return SemanticError("Unexpected node type");
}

std::optional<TypeInfo> SemanticAnalyzer::getType(const AST::Node* node, BrawContext& ctx) {
    switch (node->m_type) {
        case AST::Node::VariableDeclaration: {
            return ctx.getTypeInfo(static_cast<const AST::VariableDeclarationNode*>(node)->m_type);
        }
        case AST::Node::VariableAccess: {
            const std::string& var = static_cast<const AST::VariableAccessNode*>(node)->m_name;
            return ctx.getScopeInfo(var).value().m_type;
        }
        case AST::Node::UnaryOperator: {
            const AST::UnaryOperatorNode* op = static_cast<const AST::UnaryOperatorNode*>(node);
            auto typeOpt = getType(op->m_operand.get(), ctx);
            typeOpt = typeOpt.has_value() && op->m_operator == "->" ? Utils::getRawType(typeOpt.value(), ctx) : typeOpt;
            if(!typeOpt) return std::nullopt;

            if(op->m_operator == "&")
                return Utils::makePointer(typeOpt.value());
            else if(op->m_operator == "*")
                return Utils::getRawType(typeOpt.value(), ctx);
            else if(op->m_operator == "cast")
                return ctx.getTypeInfo(op->m_data);
            else if(op->m_operator == "." || op->m_operator == "->") {
                ScopeInfo info = ctx.getScopeInfo(op->m_data).value();
                return info.m_type;
            }
        }
        case AST::Node::BinaryOperator: {
            const AST::BinaryOperatorNode* op = static_cast<const AST::BinaryOperatorNode*>(node);
            auto leftOpt = getType(op->m_left.get(), ctx);
            auto rightOpt = getType(op->m_right.get(), ctx);
            if(!leftOpt || !rightOpt || !leftOpt->m_operators.contains(op->m_operator)) return std::nullopt;

            return ctx.getTypeInfo(leftOpt->m_operators[op->m_operator].m_returnType);
        }
        case AST::Node::FunctionCall: {
            const AST::FunctionCallNode* call = static_cast<const AST::FunctionCallNode*>(node);
            std::vector<TypeInfo> params;
            params.reserve(call->m_parameters.size());
            for(auto& param : call->m_parameters)
                params.push_back(getType(param.get(), ctx).value());
            return ctx.getFunction(call->m_name, params)->m_signature.m_returnType;
        }
        case AST::Node::Literal: {
            const AST::LiteralNode* literal = static_cast<const AST::LiteralNode*>(node);
            switch(literal->m_value.index()) {
                case 0: return ctx.m_typeTable["int"];
                case 1: return ctx.m_typeTable["long"];
                case 2: return ctx.m_typeTable["float"];
                case 3: return ctx.m_typeTable["double"];
                case 4: return ctx.m_typeTable["bool"];
                case 5: return Utils::makePointer(ctx.m_typeTable["char"]);
                case 6: return Utils::makePointer(ctx.m_typeTable["void"]);
            }
        }
        default:
            break;
    }

    return std::nullopt;
}

SemanticError SemanticAnalyzer::unknownType(const AST::Node* causer, const std::string& type) {
    return SemanticError("Unknown type: " + type, causer->m_rangeBegin, causer->m_rangeEnd);
}

SemanticError SemanticAnalyzer::mismatchedTypes(const AST::Node* causer, const std::string& type1, const std::string& type2) {
    return SemanticError(
        fmt::format("Mismatched types: {} and {}", type1, type2),
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

SemanticError SemanticAnalyzer::duplicateFunction(const AST::Node* causer, const AST::FunctionSignature& signature) {
    return SemanticError(
        fmt::format("Function {} already defined", Utils::functionSignatureString(signature)),
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

SemanticError SemanticAnalyzer::unknownVariable(const AST::VariableAccessNode* causer) {
    return SemanticError(
        fmt::format("Unknown variable: {}", causer->m_name.m_name),
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

SemanticError SemanticAnalyzer::unknownOperator(const AST::UnaryOperatorNode* causer) {
    if(causer->m_data.m_name.size() > 0) {
        return SemanticError(
            fmt::format("Unknown operator: {} ({})", causer->m_operator, causer->m_data.m_name),
            causer->m_rangeBegin,
            causer->m_rangeEnd
        );
    }

    return SemanticError(
        fmt::format("Unknown operator: {}", causer->m_operator),
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

SemanticError SemanticAnalyzer::unknownOperator(const AST::BinaryOperatorNode* causer) {
    return SemanticError(
        fmt::format("Unknown operator: {}", causer->m_operator),
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

SemanticError SemanticAnalyzer::unknownFunction(const AST::FunctionCallNode* causer, const std::vector<TypeInfo>& parameters) {
    std::string parameterString = "";
    for(int i = 0; i < causer->m_parameters.size(); i++) {
        parameterString += parameters[i].m_name;
        if(i < causer->m_parameters.size() - 1) parameterString += ", ";
    }

    return SemanticError(
        fmt::format("Unknown function: {}({})", causer->m_name.m_name, parameterString),
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}

SemanticError SemanticAnalyzer::unknownMember(const AST::Node* causer, const std::string& type, const std::string& member) {
    return SemanticError(
        fmt::format("Unknown member {} in {}", member, type),
        causer->m_rangeBegin,
        causer->m_rangeEnd
    );
}