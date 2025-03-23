#include "parser/nodes/for.hpp"
#include "parser/parser.hpp"
#include "../for.hpp"

Result<std::unique_ptr<AST::ForNode>> Parser::parseFor(TokenCursor& cursor, const std::filesystem::path& path) {
    if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
        return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD, path);

    if(!expectTokenValue(cursor.get().value(), "for"))
        return unexpectedTokenExpectedValue(cursor.value(), "for", path);

    std::unique_ptr<AST::ForNode> forNode = std::make_unique<AST::ForNode>();
    forNode->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};

    cursor.next();

    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_PAREN, path);

    auto initOpt = parseInstruction(cursor, path);
    if(!initOpt)
        return std::unexpected{initOpt.error()};
    forNode->m_initializer = std::move(initOpt.value());

    if(!expectTokenType(cursor.get().next().value(), Token::SEMICOLON))
        return unexpectedTokenExpectedType(cursor.value(), Token::SEMICOLON, path);

    auto conditionOpt = parseExpression(cursor, path);
    if(!conditionOpt)
        return std::unexpected{conditionOpt.error()};
    forNode->m_condition = std::move(conditionOpt.value());

    if(!expectTokenType(cursor.get().next().value(), Token::SEMICOLON))
        return unexpectedTokenExpectedType(cursor.value(), Token::SEMICOLON, path);

    auto incrementOpt = parseInstruction(cursor, path);
    if(!incrementOpt)
        return std::unexpected{incrementOpt.error()};
    forNode->m_increment = std::move(incrementOpt.value());

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN, path);

    auto scopeOpt = parseScope(cursor, path);
    if(!scopeOpt)
        return std::unexpected{scopeOpt.error()};
    forNode->m_body = std::move(scopeOpt.value());
    forNode->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};

    return forNode;
}