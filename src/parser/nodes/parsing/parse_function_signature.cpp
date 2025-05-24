#include "parser/parser.hpp"
#include "../function_definition.hpp"

Result<AST::FunctionSignature> Parser::parseFunctionSignature(TokenCursor& cursor, ParserContext& ctx) {
    AST::FunctionSignature sig;

    if(cursor.get().value().m_value == "ext") {
        sig.m_external = true;
        cursor.tryNext();
    }

    if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
        return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD, ctx.m_path);
    if(!expectTokenValue(cursor.get().value(), "fn"))
        return unexpectedTokenExpectedValue(cursor.value(), "fn", ctx.m_path);
    cursor.tryNext();

    auto identifierOpt = parseIdentifier(cursor, ctx);
    if(!identifierOpt) return std::unexpected{identifierOpt.error()};
    sig.m_name = identifierOpt.value();

    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_PAREN, ctx.m_path);

    while(cursor.hasNext() && cursor.get().value().m_type != Token::RIGHT_PAREN) {
        std::shared_ptr<AST::VariableDeclarationNode> var = std::make_shared<AST::VariableDeclarationNode>();
        
        auto identifierOpt = parseIdentifier(cursor, ctx);
        if(!identifierOpt) return std::unexpected{identifierOpt.error()};
        var->m_name = identifierOpt.value();

        if(!expectTokenType(cursor.get().next().value(), Token::COLON))
            return unexpectedTokenExpectedType(cursor.value(), Token::COLON, ctx.m_path);

        identifierOpt = parseTypename(cursor, ctx);
        if(!identifierOpt)
            return std::unexpected{identifierOpt.error()};
        
        var->m_type = std::move(identifierOpt.value());
        sig.m_parameters.push_back(std::move(var));

        if(cursor.get().value().m_type == Token::COMMA)
            cursor.next();
        else if(cursor.value().m_type != Token::RIGHT_PAREN)
            return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN, ctx.m_path);
    }

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN, ctx.m_path);

    if(!expectTokenValue(cursor.get().next().value(), "->"))
        return unexpectedTokenExpectedValue(cursor.value(), "->", ctx.m_path);

    identifierOpt = parseTypename(cursor, ctx);
    if(!identifierOpt)
        return std::unexpected{identifierOpt.error()};
    sig.m_returnType = identifierOpt.value();

    return sig;
}