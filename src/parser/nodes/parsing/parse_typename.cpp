#include "parser/parser.hpp"

Result<Identifier> Parser::parseTypename(TokenCursor& cursor, ParserContext& ctx) {
    if(!expectTokenTypes(cursor.get().value(), {Token::IDENTIFIER, Token::KEYWORD}))
        return unexpectedTokenExpectedTypes(cursor.value(), {Token::IDENTIFIER, Token::KEYWORD}, ctx.m_path);

    Identifier id{cursor.get().value().m_value};

    while(cursor.hasNext() && cursor.peekNext().m_value == "*") {
        id.m_name += "*";
        cursor.next();
    }

    cursor.next();

    return id;
}