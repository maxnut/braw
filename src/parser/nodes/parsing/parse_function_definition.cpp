#include "parser/parser.hpp"
#include "../function_definition.hpp"

Result<std::unique_ptr<AST::FunctionDefinitionNode>> Parser::parseFunctionDefinition(TokenCursor& cursor) {
    std::unique_ptr<AST::FunctionDefinitionNode> node = std::make_unique<AST::FunctionDefinitionNode>();
    
    auto signatureOpt = parseFunctionSignature(cursor);
    if(!signatureOpt)
        return std::unexpected{signatureOpt.error()};

    node->m_signature = std::move(signatureOpt.value());

    auto scopeOpt = parseScope(cursor);
    if(!scopeOpt)
        return std::unexpected{scopeOpt.error()};

    node->m_scope = std::move(scopeOpt.value());
    return node;
}