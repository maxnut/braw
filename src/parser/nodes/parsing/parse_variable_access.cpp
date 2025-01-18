#include "parser/parser.hpp"
#include "../variable_access.hpp"

Result<std::unique_ptr<AST::VariableAccessNode>> Parser::parseVariableAccess(TokenCursor& cursor) {
    if(!expectTokenType(cursor.get().value(), Token::IDENTIFIER))
        return unexpectedTokenExpectedType(cursor.value(), Token::IDENTIFIER);

    std::unique_ptr<AST::VariableAccessNode> variableAccess = std::make_unique<AST::VariableAccessNode>();
    variableAccess->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    variableAccess->m_name = cursor.get().next().value().m_value;
    variableAccess->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    
    return variableAccess;
}