#include "parser/parser.hpp"
#include "lexer/lexer.hpp"
#include "../file.hpp"

Result<std::unique_ptr<AST::FileNode>> Parser::parseImport(TokenCursor& cursor) {
    if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
        return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD);

    if(!expectTokenValue(cursor.get().value(), "import"))
        return unexpectedTokenExpectedValue(cursor.value(), "import");

    if(!expectTokenType(cursor.next().get().value(), Token::QUOTE))
        return unexpectedTokenExpectedType(cursor.value(), Token::QUOTE);

    if(!expectTokenType(cursor.next().get().value(), Token::STRING))
        return unexpectedTokenExpectedType(cursor.value(), Token::STRING);

    std::filesystem::path path = cursor.get().value().m_value;

    auto tokensOpt = Lexer::tokenize(path);
    if(!tokensOpt)
        return std::unexpected{ParseError{
            "Failed to tokenize file",
            cursor.get().value().m_line,
            cursor.get().value().m_column
        }};

    if(!expectTokenType(cursor.next().get().value(), Token::QUOTE))
        return unexpectedTokenExpectedType(cursor.value(), Token::QUOTE);

    cursor.tryNext();

    return Parser::parse(tokensOpt.value());
}