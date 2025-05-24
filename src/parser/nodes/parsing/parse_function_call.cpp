#include "parser/parser.hpp"
#include "../function_call.hpp"

Result<std::shared_ptr<AST::FunctionCallNode>> Parser::parseFunctionCall(TokenCursor& cursor, ParserContext& ctx) {
    if(!expectTokenType(cursor.get().value(), Token::IDENTIFIER))
        return unexpectedTokenExpectedType(cursor.value(), Token::IDENTIFIER, ctx.m_path);

    std::shared_ptr<AST::FunctionCallNode> functionCall = std::make_shared<AST::FunctionCallNode>();
    functionCall->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    auto identifierOpt = parseIdentifier(cursor, ctx);
    if(!identifierOpt)
        return std::unexpected{identifierOpt.error()};
    functionCall->m_name = std::move(identifierOpt.value());

    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_PAREN, ctx.m_path);

    while(cursor.hasNext() && cursor.get().value().m_type != Token::RIGHT_PAREN) {
        auto paramOpt = parseExpression(cursor, ctx);
        if(!paramOpt)
            return std::unexpected{paramOpt.error()};
        functionCall->m_parameters.push_back(std::move(paramOpt.value()));

        if(cursor.get().value().m_type == Token::RIGHT_PAREN)
            break;

        if(!expectTokenType(cursor.get().next().value(), Token::COMMA))
            return unexpectedTokenExpectedType(cursor.value(), Token::COMMA, ctx.m_path);
    }

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN, ctx.m_path);

    functionCall->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    
    return functionCall;
}