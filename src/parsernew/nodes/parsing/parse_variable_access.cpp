#include "parsernew/parser.hpp"
#include "../variable_access.hpp"

Result<std::shared_ptr<AST::VariableAccessNode>> Parser::parseVariableAccess(TokenCursor& cursor) {
    if(!expectTokenType(cursor.get().value(), Token::IDENTIFIER))
        return unexpectedTokenExpectedType(cursor.value(), Token::IDENTIFIER);

    std::shared_ptr<AST::VariableAccessNode> variableAccess = std::make_shared<AST::VariableAccessNode>();
    variableAccess->m_name = cursor.get().next().value().m_value;
    
    return variableAccess;
}