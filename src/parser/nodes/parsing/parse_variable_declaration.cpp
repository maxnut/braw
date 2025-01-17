#include "parser/parser.hpp"
#include "../variable_declaration.hpp"

Result<std::unique_ptr<AST::Node>> Parser::parseVariableDeclaration(TokenCursor& cursor) {
    std::unique_ptr<AST::Node> ret = nullptr;

    bool assignment = Rules::isAssignment(cursor);
    std::unique_ptr<AST::VariableDeclarationNode> variableDeclaration = std::make_unique<AST::VariableDeclarationNode>();
    variableDeclaration->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};

    auto typeOpt = parseTypename(cursor);
    if(!typeOpt)
        return std::unexpected{typeOpt.error()};

    variableDeclaration->m_type = typeOpt.value();
    variableDeclaration->m_name = cursor.get().next().value().m_value;

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