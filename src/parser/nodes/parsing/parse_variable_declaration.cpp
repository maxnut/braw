#include "parser/parser.hpp"
#include "../variable_declaration.hpp"

Result<std::unique_ptr<AST::Node>> Parser::parseVariableDeclaration(TokenCursor& cursor) {
    std::unique_ptr<AST::Node> ret = nullptr;

    std::unique_ptr<AST::VariableDeclarationNode> variableDeclaration = std::make_unique<AST::VariableDeclarationNode>();
    variableDeclaration->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};

    if(!expectTokenType(cursor.get().value(), Token::KEYWORD))
        return unexpectedTokenExpectedType(cursor.value(), Token::KEYWORD);
    if(!expectTokenValue(cursor.get().value(), "let"))
        return unexpectedTokenExpectedValue(cursor.value(), "let");

    variableDeclaration->m_name = cursor.next().get().next().value().m_value;

    if(!expectTokenType(cursor.get().next().value(), Token::COLON))
        return unexpectedTokenExpectedType(cursor.value(), Token::COLON);

    auto typeOpt = parseTypename(cursor);
    if(!typeOpt)
        return std::unexpected{typeOpt.error()};

    variableDeclaration->m_type = typeOpt.value();
    bool assignment = Rules::isAssignment(cursor);
    if(assignment) {
        cursor.next();
        auto exprOpt = parseExpression(cursor);
        if(!exprOpt)
            return std::unexpected{exprOpt.error()};

        variableDeclaration->m_value = std::move(exprOpt.value());
    }

    variableDeclaration->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    ret = std::move(variableDeclaration);

    return ret;
}