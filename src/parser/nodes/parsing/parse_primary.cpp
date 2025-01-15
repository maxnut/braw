#include "parser/parser.hpp"
#include "../function_call.hpp"
#include "../variable_access.hpp"
#include "../literal.hpp"
#include "../unary_operator.hpp"

Result<std::unique_ptr<AST::Node>> Parser::parsePrimary(TokenCursor& cursor) {
    Result<std::unique_ptr<AST::Node>> result;
    Token beg = cursor.get().value();

    if(Rules::isFunctionCall(cursor))
        result = parseFunctionCall(cursor);
    else if(Rules::isVariableAccess(cursor))
        result = parseVariableAccess(cursor);
    else if(Rules::isLiteral(cursor))
        result = parseLiteral(cursor);
    else if(Rules::isCast(cursor))
        result = parseCast(cursor);
    else if(beg.m_type == Token::LEFT_PAREN) {
        cursor.next();
        result = parseExpression(cursor);
        if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
            return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN);
    }

    return result;
}