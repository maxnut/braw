#include "parser/parser.hpp"
#include "../return.hpp"

Result<std::shared_ptr<AST::ReturnNode>> Parser::parseReturn(TokenCursor& cursor, ParserContext& ctx) {
    std::shared_ptr<AST::ReturnNode> returnNode = std::make_shared<AST::ReturnNode>();
    returnNode->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};

    if(cursor.next().get().value().m_type != Token::SEMICOLON) {
        auto valueOpt = parseExpression(cursor, ctx);
        if(!valueOpt)
            return std::unexpected{valueOpt.error()};
        returnNode->m_value = std::move(valueOpt.value());
    }

    if(!expectTokenType(cursor.get().value(), Token::SEMICOLON))
        return unexpectedTokenExpectedType(cursor.value(), Token::SEMICOLON, ctx.m_path);

    returnNode->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    return returnNode;
}