#include "parsernew/parser.hpp"

Result<Identifier> Parser::parseTypename(TokenCursor& cursor) {
    if(!expectTokenType(cursor.get().value(), Token::IDENTIFIER))
        return unexpectedTokenExpectedType(cursor.value(), Token::IDENTIFIER);

    Identifier id{cursor.get().value().m_value};

    while(cursor.hasNext() && cursor.peekNext().m_value == "*") {
        id.m_name += "*";
        cursor.next();
    }

    cursor.next();

    return id;
}