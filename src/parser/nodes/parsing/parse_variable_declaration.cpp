#include "parser/parser.hpp"
#include "../variable_declaration.hpp"

Result<std::unique_ptr<AST::Node>> Parser::parseVariableDeclaration(TokenCursor& cursor, const std::filesystem::path& path) {
    std::unique_ptr<AST::Node> ret = nullptr;

    std::unique_ptr<AST::VariableDeclarationNode> variableDeclaration = std::make_unique<AST::VariableDeclarationNode>();
    variableDeclaration->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};

    if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
        return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD, path);
    if(!expectTokenValue(cursor.get().value(), "let"))
        return unexpectedTokenExpectedValue(cursor.value(), "let", path);

    variableDeclaration->m_name = cursor.next().get().next().value().m_value;

    if(cursor.get().value().m_type == Token::LEFT_BRACKET) {
        if(!expectTokenTypes(cursor.next().get().value(), {Token::INTEGER, Token::LONG}))
            return unexpectedTokenExpectedTypes(cursor.value(), {Token::INTEGER, Token::LONG}, path);
        variableDeclaration->m_scale = std::stoul(cursor.value().m_value);
        if(!expectTokenType(cursor.next().get().next().value(), Token::RIGHT_BRACKET))
            return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_BRACE, path);
    }

    if(!expectTokenType(cursor.get().next().value(), Token::COLON))
        return unexpectedTokenExpectedType(cursor.value(), Token::COLON, path);

    auto typeOpt = parseTypename(cursor, path);
    if(!typeOpt)
        return std::unexpected{typeOpt.error()};

    variableDeclaration->m_type = typeOpt.value();
    bool assignment = Rules::isAssignment(cursor);
    if(assignment) {
        cursor.next();
        auto exprOpt = parseExpression(cursor, path);
        if(!exprOpt)
            return std::unexpected{exprOpt.error()};

        variableDeclaration->m_value = std::move(exprOpt.value());
    }

    variableDeclaration->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    ret = std::move(variableDeclaration);

    return ret;
}