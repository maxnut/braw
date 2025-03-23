#include "parser/identifier.hpp"
#include "parser/parser.hpp"
#include "../function_definition.hpp"

Result<AST::FunctionSignature> Parser::parseFunctionSignature(TokenCursor& cursor, const std::filesystem::path& path, std::shared_ptr<AST::FileNode> file) {
    AST::FunctionSignature sig;

    if(cursor.get().value().m_value == "ext") {
        sig.m_external = true;
        cursor.tryNext();
    }

    if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
        return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD, path);
    if(!expectTokenValue(cursor.get().value(), "fn"))
        return unexpectedTokenExpectedValue(cursor.value(), "fn", path);

    sig.m_name = cursor.next().get().value().m_value;

    if(!expectTokenType(cursor.next().get().next().value(), Token::LEFT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_PAREN, path);

    while(cursor.hasNext() && cursor.get().value().m_type != Token::RIGHT_PAREN) {
        std::shared_ptr<AST::VariableDeclarationNode> var = std::make_shared<AST::VariableDeclarationNode>();
        
        var->m_name = cursor.get().next().value().m_value;

        if(!expectTokenType(cursor.get().next().value(), Token::COLON))
            return unexpectedTokenExpectedType(cursor.value(), Token::COLON, path);

        Result<Identifier> typeRes = parseTypename(cursor, path, file);
        if(!typeRes)
            return std::unexpected{typeRes.error()};
        
        var->m_type = typeRes.value();
        sig.m_parameters.push_back(std::move(var));

        if(cursor.get().value().m_type == Token::COMMA)
            cursor.next();
        else if(cursor.value().m_type != Token::RIGHT_PAREN)
            return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN, path);
    }

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN, path);

    if(!expectTokenType(cursor.get().value(), Token::OPERATOR))
        return unexpectedTokenExpectedType(cursor.value(), Token::OPERATOR, path);
    if(!expectTokenValue(cursor.get().next().value(), "->"))
        return unexpectedTokenExpectedValue(cursor.value(), "->", path);

    Result<Identifier> typeRes = parseTypename(cursor, path, file);
    if(!typeRes)
        return std::unexpected{typeRes.error()};
    sig.m_returnType = typeRes.value();

    return sig;
}