#include "parsernew/parser.hpp"
#include "../unary_operator.hpp"

Result<std::shared_ptr<AST::UnaryOperatorNode>> Parser::parseCast(TokenCursor& cursor) {
    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_PAREN);

    std::shared_ptr<AST::UnaryOperatorNode> cast = std::make_shared<AST::UnaryOperatorNode>();
    cast->m_operator = "cast";

    auto typeOpt = parseTypename(cursor);
    if(!typeOpt)
        return std::unexpected{typeOpt.error()};
    cast->m_data = typeOpt.value().m_name;

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN);

    auto baseOpt = parseOperand(cursor);
    if(!baseOpt)
        return std::unexpected{baseOpt.error()};
    cast->m_operand = std::move(baseOpt.value());

    return cast;
}