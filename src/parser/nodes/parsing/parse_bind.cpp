#include "parser/parser.hpp"
#include "../bind.hpp"

Result<std::unique_ptr<AST::BindNode>> Parser::parseBind(TokenCursor& cursor) {
    if(!expectTokenType(cursor.get().next().value(), Token::KEYWORD))
        return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD);
    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_PAREN);
    if(!expectTokenType(cursor.get().next().value(), Token::QUOTE))
        return unexpectedTokenExpectedType(cursor.value(), Token::QUOTE);
    if(!expectTokenType(cursor.get().value(), Token::STRING))
        return unexpectedTokenExpectedType(cursor.value(), Token::STRING);

    std::unique_ptr<AST::BindNode> bind = std::make_unique<AST::BindNode>();

    bind->m_library = cursor.value().m_value;

    if(!expectTokenType(cursor.next().get().value(), Token::QUOTE))
        return unexpectedTokenExpectedType(cursor.value(), Token::QUOTE);
    if(!expectTokenType(cursor.next().get().value(), Token::RIGHT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN);
    if(!expectTokenType(cursor.next().get().next().value(), Token::LEFT_BRACE))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_BRACE);

    while(cursor.hasNext() && cursor.get().value().m_type != Token::RIGHT_BRACE) {
        if(!expectTokenType(cursor.get().next().value(), Token::QUOTE))
            return unexpectedTokenExpectedType(cursor.value(), Token::QUOTE);
        if(!expectTokenType(cursor.get().value(), Token::STRING))
            return unexpectedTokenExpectedType(cursor.value(), Token::STRING);

        std::pair<Identifier, AST::FunctionSignature> binding;
        binding.first = cursor.get().next().value().m_value;
        
        if(!expectTokenType(cursor.get().next().value(), Token::QUOTE))
            return unexpectedTokenExpectedType(cursor.value(), Token::QUOTE);

        auto signatureOpt = parseFunctionSignature(cursor);
        if(!signatureOpt)
            return std::unexpected{signatureOpt.error()};

        binding.second = signatureOpt.value();

        if(!expectTokenType(cursor.get().next().value(), Token::SEMICOLON))
            return unexpectedTokenExpectedType(cursor.value(), Token::SEMICOLON);

        bind->m_functions.push_back(binding);
    }

    if(!expectTokenType(cursor.get().value(), Token::RIGHT_BRACE))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_BRACE);

    cursor.tryNext();

    return bind;
}