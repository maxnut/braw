#include "parsernew/parser.hpp"
#include "../function_definition.hpp"

Result<AST::FunctionSignature> Parser::parseFunctionSignature(TokenCursor& cursor) {
    AST::FunctionSignature sig;
    
    auto typeRes = parseTypename(cursor);
    if(!typeRes)
        return std::unexpected{typeRes.error()};
    
    sig.m_returnType = typeRes.value();
    sig.m_name = cursor.get().value().m_value;

    if(!expectTokenType(cursor.next().get().next().value(), Token::LEFT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_PAREN);

    while(cursor.hasNext() && cursor.get().value().m_type != Token::RIGHT_PAREN) {
        AST::VariableDeclarationNode var;
        typeRes = parseTypename(cursor);
        if(!typeRes)
            return std::unexpected{typeRes.error()};
        
        var.m_type = typeRes.value();
        
        if(cursor.get().value().m_type == Token::IDENTIFIER)
            var.m_name = cursor.get().next().value().m_value;

        sig.m_parameters.push_back(var);

        if(cursor.get().value().m_type == Token::COMMA)
            cursor.next();
        else if(cursor.value().m_type != Token::RIGHT_PAREN)
            return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN);
    }

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN);

    return sig;
}