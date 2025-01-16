#include "parser/parser.hpp"
#include "../scope.hpp"

Result<std::unique_ptr<AST::ScopeNode>> Parser::parseScope(TokenCursor& cursor) {
    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_BRACE))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_BRACE);

    std::unique_ptr<AST::ScopeNode> scope = std::make_unique<AST::ScopeNode>();

    while(cursor.hasNext() && cursor.get().value().m_type != Token::RIGHT_BRACE) {
        auto instructionOpt = parseInstruction(cursor);
        if(!instructionOpt)
            return std::unexpected{instructionOpt.error()};
        scope->m_instructions.push_back(std::move(instructionOpt.value()));
    }

    if(!expectTokenType(cursor.get().value(), Token::RIGHT_BRACE))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_BRACE);

    cursor.tryNext();

    return scope;
}