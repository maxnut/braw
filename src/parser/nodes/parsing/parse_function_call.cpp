#include "parser/parser.hpp"
#include "../function_call.hpp"

Result<std::shared_ptr<AST::FunctionCallNode>> Parser::parseFunctionCall(TokenCursor& cursor, const std::filesystem::path& path, std::shared_ptr<AST::FileNode> file) {
    if(!expectTokenType(cursor.get().value(), Token::IDENTIFIER))
        return unexpectedTokenExpectedType(cursor.value(), Token::IDENTIFIER, path);

    std::shared_ptr<AST::FunctionCallNode> functionCall = std::make_shared<AST::FunctionCallNode>();
    functionCall->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    functionCall->m_name = cursor.value().m_value;

    if(!expectTokenType(cursor.next().get().next().value(), Token::LEFT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_PAREN, path);

    while(cursor.hasNext() && cursor.get().value().m_type != Token::RIGHT_PAREN) {
        auto paramOpt = parseExpression(cursor, path, file);
        if(!paramOpt)
            return std::unexpected{paramOpt.error()};
        functionCall->m_parameters.push_back(std::move(paramOpt.value()));

        if(cursor.get().value().m_type == Token::RIGHT_PAREN)
            break;

        if(!expectTokenType(cursor.get().next().value(), Token::COMMA))
            return unexpectedTokenExpectedType(cursor.value(), Token::COMMA, path);
    }

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN, path);

    functionCall->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    
    return functionCall;
}