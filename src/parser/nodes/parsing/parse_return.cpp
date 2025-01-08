#include "parser/parser.hpp"
#include "../return.hpp"

std::unique_ptr<FunctionInstructionNode> Parser::parseReturn(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    std::unique_ptr<ReturnNode> returnNode = std::make_unique<ReturnNode>();

    if(cursor.next().get().value().m_type != Token::SEMICOLON) {
        returnNode->m_value = parseExpression(file, cursor, ctx, 0);
        if(!returnNode->m_value) return nullptr;
    }

    if(!expectTokenType(cursor.get().value(), Token::SEMICOLON))
        return nullptr;

    return returnNode;
}