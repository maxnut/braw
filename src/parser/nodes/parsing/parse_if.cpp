#include "parser/parser.hpp"
#include "../if.hpp"

std::unique_ptr<FunctionInstructionNode> Parser::parseIf(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    if(!expectTokenType(cursor.get().value(), Token::KEYWORD) || !expectTokenValue(cursor.get().value(), "if"))
        return nullptr;
    cursor.next();

    std::unique_ptr<IfNode> ifNode = std::make_unique<IfNode>();

    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_PAREN))
        return nullptr;

    ifNode->m_condition = parseExpression(file, cursor, ctx, 0);
    if(!ifNode->m_condition)
        return nullptr;

    if(ifNode->m_condition->m_type.m_name != "bool") {
        m_message.expectedType(ifNode->m_condition->m_type.m_name, "bool");
        return nullptr;
    }

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return nullptr;

    ifNode->m_scope = parseScope(file, cursor, ctx);
    if(!ifNode->m_scope)
        return nullptr;

    return ifNode;
}