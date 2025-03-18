#include "parser/parser.hpp"
#include "../function_definition.hpp"

Result<std::unique_ptr<AST::FunctionDefinitionNode>> Parser::parseFunctionDefinition(TokenCursor& cursor, const std::filesystem::path& path) {
    std::unique_ptr<AST::FunctionDefinitionNode> node = std::make_unique<AST::FunctionDefinitionNode>();
    node->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    
    auto signatureOpt = parseFunctionSignature(cursor, path);
    if(!signatureOpt)
        return std::unexpected{signatureOpt.error()};

    node->m_signature = std::move(signatureOpt.value());

    if(node->m_signature.m_external) {
        if(!expectTokenType(cursor.get().value(), Token::SEMICOLON))
            return unexpectedTokenExpectedType(cursor.value(), Token::SEMICOLON, path);
        cursor.tryNext();
    }
    else {
        auto scopeOpt = parseScope(cursor, path);
        if(!scopeOpt)
            return std::unexpected{scopeOpt.error()};
        node->m_scope = std::move(scopeOpt.value());
    }

    node->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    return node;
}