#include "parser/parser.hpp"
#include "../function_call.hpp"
#include "../variable_access.hpp"
#include "../literal.hpp"
#include "../unary_operator.hpp"

Result<std::shared_ptr<AST::Node>> Parser::parsePrimary(TokenCursor& cursor, const std::filesystem::path& path, std::shared_ptr<AST::FileNode> file) {
    Result<std::shared_ptr<AST::Node>> result;
    Token beg = cursor.get().value();

    if(Rules::isFunctionCall(cursor))
        result = parseFunctionCall(cursor, path, file);
    else if(Rules::isVariableAccess(cursor))
        result = parseVariableAccess(cursor, path, file);
    else if(Rules::isLiteral(cursor))
        result = parseLiteral(cursor, path, file);
    else if(Rules::isCast(cursor))
        result = parseCast(cursor, path, file);
    else if(beg.m_type == Token::LEFT_PAREN) {
        cursor.next();
        result = parseExpression(cursor, path, file);
        if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
            return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN, path);
    }
    else
        result = unexpectedToken(beg, path);

    return result;
}