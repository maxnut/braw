#include "parser/parser.hpp"
#include "../return.hpp"

Result<std::unique_ptr<AST::ReturnNode>> Parser::parseReturn(TokenCursor& cursor, const std::filesystem::path& path) {
    std::unique_ptr<AST::ReturnNode> returnNode = std::make_unique<AST::ReturnNode>();
    returnNode->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};

    if(cursor.next().get().value().m_type != Token::SEMICOLON) {
        auto valueOpt = parseExpression(cursor, path);
        if(!valueOpt)
            return std::unexpected{valueOpt.error()};
        returnNode->m_value = std::move(valueOpt.value());
    }

    if(!expectTokenType(cursor.get().value(), Token::SEMICOLON))
        return unexpectedTokenExpectedType(cursor.value(), Token::SEMICOLON, path);

    returnNode->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    return returnNode;
}