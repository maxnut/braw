#include "parser/parser.hpp"
#include "../scope.hpp"

Result<std::unique_ptr<AST::ScopeNode>> Parser::parseScope(TokenCursor& cursor, bool allowOneLine) {
    std::unique_ptr<AST::ScopeNode> scope = std::make_unique<AST::ScopeNode>();
    scope->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};

    bool oneLine = cursor.get().value().m_type != Token::LEFT_BRACE && allowOneLine;

    if(!oneLine && !expectTokenType(cursor.get().next().value(), Token::LEFT_BRACE))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_BRACE);

    while(cursor.hasNext() && cursor.get().value().m_type != Token::RIGHT_BRACE) {
        auto instructionOpt = parseInstruction(cursor);
        if(!instructionOpt)
            return std::unexpected{instructionOpt.error()};
        scope->m_instructions.push_back(std::move(instructionOpt.value()));

        if(oneLine) break;
    }

    if(!oneLine && !expectTokenType(cursor.get().value(), Token::RIGHT_BRACE))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_BRACE);

    scope->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};

    if(!oneLine)
        cursor.tryNext();

    return scope;
}