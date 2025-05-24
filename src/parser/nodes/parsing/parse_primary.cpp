#include "parser/parser.hpp"
#include "../function_call.hpp"
#include "../variable_access.hpp"
#include "../literal.hpp"
#include "../unary_operator.hpp"

Result<std::shared_ptr<AST::Node>> Parser::parsePrimary(TokenCursor& cursor, ParserContext& ctx) {
    if(Rules::isMacroCall(cursor))
        return parseMacroCall(cursor, ctx);
    else if(Rules::isMacroParameterReference(cursor))
        return parseMacroParameter(cursor, ctx);

    Result<std::shared_ptr<AST::Node>> result;
    Token beg = cursor.get().value();

    if(Rules::isFunctionCall(cursor))
        result = parseFunctionCall(cursor, ctx);
    else if(Rules::isVariableAccess(cursor))
        result = parseVariableAccess(cursor, ctx); //remove variableaccessnode?
    else if(Rules::isLiteral(cursor))
        result = parseLiteral(cursor, ctx);
    else if(Rules::isCast(cursor))
        result = parseCast(cursor, ctx);
    else if(beg.m_type == Token::LEFT_PAREN) {
        cursor.next();
        result = parseExpression(cursor, ctx);
        if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
            return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN, ctx.m_path);
    }
    else
        result = unexpectedToken(beg, ctx.m_path);

    return result;
}