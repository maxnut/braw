#include "parser/parser.hpp"
#include "../unary_operator.hpp"

Result<std::unique_ptr<AST::UnaryOperatorNode>> Parser::parseCast(TokenCursor& cursor) {
    std::unique_ptr<AST::UnaryOperatorNode> cast = std::make_unique<AST::UnaryOperatorNode>();
    cast->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_PAREN);

    cast->m_operator = "cast";

    auto typeOpt = parseTypename(cursor);
    if(!typeOpt)
        return std::unexpected{typeOpt.error()};
    cast->m_data = typeOpt.value();

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN);

    auto baseOpt = parseOperand(cursor);
    if(!baseOpt)
        return std::unexpected{baseOpt.error()};
    cast->m_operand = std::move(baseOpt.value());
    cast->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};

    return cast;
}