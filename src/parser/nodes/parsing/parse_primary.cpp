#include "parser/parser.hpp"

std::unique_ptr<EvaluatableNode> Parser::parsePrimary(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    std::unique_ptr<EvaluatableNode> result = nullptr;
    Token beg = cursor.get().value();

    if(Rules::isFunctionCall(cursor))
        result = parseFunctionCall(file, cursor, ctx);
    else if(Rules::isVariableAccess(cursor))
        result = parseVariableAccess(file, cursor, ctx);
    else if(Rules::isLiteral(cursor))
        result = parseLiteral(file, cursor, ctx);
    else if(Rules::isCast(cursor))
        result = parseCast(file, cursor, ctx);
    else if(beg.m_type == Token::LEFT_PAREN) {
        cursor.next();
        result = parseExpression(file, cursor, ctx);
        if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
            return nullptr;
    }
    
    if(!result)
        m_message.unexpectedToken(beg);

    return result;
}