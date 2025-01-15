#include "parsernew/parser.hpp"
#include "../variable_declaration.hpp"
#include "../binary_operator.hpp"

Result<std::shared_ptr<AST::Node>> Parser::parseVariableDeclaration(TokenCursor& cursor) {
    std::shared_ptr<AST::Node> ret = nullptr;

    bool assignment = Rules::isAssignment(cursor);

    auto typeOpt = parseTypename(cursor);
    if(!typeOpt)
        return std::unexpected{typeOpt.error()};

    std::shared_ptr<AST::VariableDeclarationNode> variableDeclaration = std::make_shared<AST::VariableDeclarationNode>();
    variableDeclaration->m_type = typeOpt.value();
    variableDeclaration->m_name = cursor.get().next().value().m_value;

    ret = std::move(variableDeclaration);

    if(assignment) {
        std::shared_ptr<AST::BinaryOperatorNode> equals = std::make_shared<AST::BinaryOperatorNode>();
        equals->m_operator = "=";
        equals->m_left = std::move(variableDeclaration);

        cursor.next();
        auto exprOpt = parseExpression(cursor);
        if(!exprOpt)
            return std::unexpected{exprOpt.error()};

        equals->m_right = std::move(exprOpt.value());

        ret = std::move(equals);
    }

    return ret;
}