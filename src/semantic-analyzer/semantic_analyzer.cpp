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
        case AST::Node::Type::File:
            return analyze(static_cast<const AST::FileNode*>(root), context);
        case AST::Node::Type::FunctionDefinition:
            return analyze(static_cast<const AST::FunctionDefinitionNode*>(root), context);
        case AST::Node::Type::Scope:
            return analyze(static_cast<const AST::ScopeNode*>(root), context);
        case AST::Node::Type::VariableDeclaration:
            return analyze(static_cast<const AST::VariableDeclarationNode*>(root), context);
        case AST::Node::Type::VariableAccess:
            return analyze(static_cast<const AST::VariableAccessNode*>(root), context);
        case AST::Node::Type::UnaryOperator:
            return analyze(static_cast<const AST::UnaryOperatorNode*>(root), context);
        case AST::Node::Type::BinaryOperator:
            return analyze(static_cast<const AST::BinaryOperatorNode*>(root), context);
        case AST::Node::Type::Struct:
            return analyze(static_cast<const AST::StructNode*>(root), context);
        case AST::Node::Type::FunctionCall:
            return analyze(static_cast<const AST::FunctionCallNode*>(root), context);
        case AST::Node::Type::Bind:
            return analyze(static_cast<const AST::BindNode*>(root), context);
        case AST::Node::Type::If:
            return analyze(static_cast<const AST::IfNode*>(root), context);
        case AST::Node::Type::While:
            return analyze(static_cast<const AST::WhileNode*>(root), context);
        case AST::Node::Type::Literal:
            return analyze(static_cast<const AST::LiteralNode*>(root), context);
        case AST::Node::Type::Return:
            return analyze(static_cast<const AST::ReturnNode*>(root), context);
    }
    return SemanticError("Unexpected node type");
}

std::optional<TypeInfo> SemanticAnalyzer::getType(const AST::Node* node, BrawContext& ctx) {
    switch (node->m_type) {
        case AST::Node::Type::VariableAccess: {
            const std::string& var = static_cast<const AST::VariableAccessNode*>(node)->m_name;
            return ctx.getScopeInfo(var).value().m_type;
        }
        case AST::Node::Type::UnaryOperator: {
            
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

SemanticError SemanticAnalyzer::duplicateFunction(const AST::FunctionDefinitionNode* causer) {
    return SemanticError(
        fmt::format("Function {} already defined", Utils::functionSignatureString(causer->m_signature)),
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