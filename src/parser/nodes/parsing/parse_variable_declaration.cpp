#include "parser/parser.hpp"
#include "../variable_declaration.hpp"
#include "../binary_operator.hpp"

Result<std::unique_ptr<AST::Node>> Parser::parseVariableDeclaration(TokenCursor& cursor) {
    std::unique_ptr<AST::Node> ret = nullptr;

    bool assignment = Rules::isAssignment(cursor);

    auto typeOpt = parseTypename(cursor);
    if(!typeOpt)
        return std::unexpected{typeOpt.error()};

    std::unique_ptr<AST::VariableDeclarationNode> variableDeclaration = std::make_unique<AST::VariableDeclarationNode>();
    variableDeclaration->m_type = typeOpt.value();
    variableDeclaration->m_name = cursor.get().next().value().m_value;

    ret = std::move(variableDeclaration);

    if(assignment) {
        std::unique_ptr<AST::BinaryOperatorNode> equals = std::make_unique<AST::BinaryOperatorNode>();
        equals->m_operator = "=";
        equals->m_left = std::move(ret);

        cursor.next();
        auto exprOpt = parseExpression(cursor);
        if(!exprOpt)
            return std::unexpected{exprOpt.error()};

        equals->m_right = std::move(exprOpt.value());

        ret = std::move(equals);
    }

    return ret;
}