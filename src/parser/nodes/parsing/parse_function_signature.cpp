#include "parser/identifier.hpp"
#include "parser/parser.hpp"
#include "../function_definition.hpp"

Result<AST::FunctionSignature> Parser::parseFunctionSignature(TokenCursor& cursor) {
    AST::FunctionSignature sig;

    if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
        return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD);
    if(!expectTokenValue(cursor.get().value(), "fn"))
        return unexpectedTokenExpectedValue(cursor.value(), "fn");

    sig.m_name = cursor.next().get().value().m_value;

    if(!expectTokenType(cursor.next().get().next().value(), Token::LEFT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_PAREN);

    while(cursor.hasNext() && cursor.get().value().m_type != Token::RIGHT_PAREN) {
        std::unique_ptr<AST::VariableDeclarationNode> var = std::make_unique<AST::VariableDeclarationNode>();
        
        var->m_name = cursor.get().next().value().m_value;

        if(!expectTokenType(cursor.get().next().value(), Token::COLON))
            return unexpectedTokenExpectedType(cursor.value(), Token::COLON);

        Result<Identifier> typeRes = parseTypename(cursor);
        if(!typeRes)
            return std::unexpected{typeRes.error()};
        
        var->m_type = typeRes.value();
        sig.m_parameters.push_back(std::move(var));

        if(cursor.get().value().m_type == Token::COMMA)
            cursor.next();
        else if(cursor.value().m_type != Token::RIGHT_PAREN)
            return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN);
    }

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN);

    if(!expectTokenType(cursor.get().value(), Token::OPERATOR))
        return unexpectedTokenExpectedType(cursor.value(), Token::OPERATOR);
    if(!expectTokenValue(cursor.get().next().value(), "->"))
        return unexpectedTokenExpectedValue(cursor.value(), "->");

    Result<Identifier> typeRes = parseTypename(cursor);
    if(!typeRes)
        return std::unexpected{typeRes.error()};
    sig.m_returnType = typeRes.value();

    return sig;
}