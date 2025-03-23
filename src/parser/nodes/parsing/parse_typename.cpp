#include "parser/parser.hpp"

Result<Identifier> Parser::parseTypename(TokenCursor& cursor, const std::filesystem::path& path, std::shared_ptr<AST::FileNode> file) {
    if(!expectTokenTypes(cursor.get().value(), {Token::IDENTIFIER, Token::KEYWORD}))
        return unexpectedTokenExpectedTypes(cursor.value(), {Token::IDENTIFIER, Token::KEYWORD}, path);

    Identifier id{cursor.get().value().m_value};

    while(cursor.hasNext() && cursor.peekNext().m_value == "*") {
        id.m_name += "*";
        cursor.next();
    }

    cursor.next();

    return id;
}