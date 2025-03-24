#include "parser/parser.hpp"
#include "../variable_declaration.hpp"

Result<std::shared_ptr<AST::VariableDeclarationNode>> Parser::parseVariableDeclaration(TokenCursor& cursor, ParserContext& ctx, bool omitLet) {
    std::shared_ptr<AST::VariableDeclarationNode> variableDeclaration = std::make_shared<AST::VariableDeclarationNode>();
    variableDeclaration->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};

    if(!omitLet) {
        if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
            return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD, ctx.m_path);
        if(!expectTokenValue(cursor.get().next().value(), "let"))
            return unexpectedTokenExpectedValue(cursor.value(), "let", ctx.m_path);
    }

    variableDeclaration->m_name = cursor.get().next().value().m_value;

    if(cursor.get().value().m_type == Token::LEFT_BRACKET) {
        if(!expectTokenTypes(cursor.next().get().value(), {Token::INTEGER, Token::LONG}))
            return unexpectedTokenExpectedTypes(cursor.value(), {Token::INTEGER, Token::LONG}, ctx.m_path);
        variableDeclaration->m_scale = std::stoul(cursor.value().m_value);
        if(!expectTokenType(cursor.next().get().next().value(), Token::RIGHT_BRACKET))
            return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_BRACE, ctx.m_path);
    }

    if(!expectTokenType(cursor.get().next().value(), Token::COLON))
        return unexpectedTokenExpectedType(cursor.value(), Token::COLON, ctx.m_path);

    auto typeOpt = parseTypename(cursor, ctx);
    if(!typeOpt)
        return std::unexpected{typeOpt.error()};

    variableDeclaration->m_type = typeOpt.value();
    bool assignment = Rules::isAssignment(cursor);
    if(assignment) {
        cursor.next();
        auto exprOpt = parseExpression(cursor, ctx);
        if(!exprOpt)
            return std::unexpected{exprOpt.error()};

        variableDeclaration->m_value = std::move(exprOpt.value());
    }

    variableDeclaration->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};

    return variableDeclaration;
}