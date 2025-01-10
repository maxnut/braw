#include "parser/parser.hpp"
#include "../while.hpp"

std::unique_ptr<FunctionInstructionNode> Parser::parseWhile(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    if(!expectTokenType(cursor.get().value(), Token::KEYWORD) || !expectTokenValue(cursor.get().value(), "while"))
        return nullptr;
    cursor.next();

    std::unique_ptr<WhileNode> whileNode = std::make_unique<WhileNode>();

    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_PAREN))
        return nullptr;

    whileNode->m_condition = parseExpression(file, cursor, ctx, 0);
    if(!whileNode->m_condition)
        return nullptr;

    if(whileNode->m_condition->m_type.m_name != "bool") {
        m_message.expectedType(whileNode->m_condition->m_type.m_name, "bool");
        return nullptr;
    }

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return nullptr;

    whileNode->m_scope = parseScope(file, cursor, ctx);
    if(!whileNode->m_scope)
        return nullptr;

    return whileNode;
}