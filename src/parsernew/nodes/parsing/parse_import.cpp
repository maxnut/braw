#include "parsernew/parser.hpp"
#include "../import.hpp"

Result<std::shared_ptr<AST::ImportNode>> Parser::parseImport(TokenCursor& cursor) {
    if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
        return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD);

    if(!expectTokenValue(cursor.get().value(), "import"))
        return unexpectedTokenExpectedValue(cursor.value(), "import");

    std::shared_ptr<AST::ImportNode> importNode = std::make_shared<AST::ImportNode>();

    if(!expectTokenType(cursor.next().get().value(), Token::QUOTE))
        return unexpectedTokenExpectedType(cursor.value(), Token::QUOTE);

    if(!expectTokenType(cursor.next().get().value(), Token::STRING))
        return unexpectedTokenExpectedType(cursor.value(), Token::STRING);

    importNode->m_path = cursor.get().value().m_value;

    if(!expectTokenType(cursor.next().get().value(), Token::QUOTE))
        return unexpectedTokenExpectedType(cursor.value(), Token::QUOTE);

    cursor.tryNext();

    return importNode;
}