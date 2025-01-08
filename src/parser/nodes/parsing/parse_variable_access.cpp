#include "parser/parser.hpp"
#include "../variable_access.hpp"

std::unique_ptr<EvaluatableNode> Parser::parseVariableAccess(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    if(!expectTokenType(cursor.get().value(), Token::IDENTIFIER))
        return nullptr;

    std::string name = cursor.get().next().value().m_value;
    std::optional<ScopeInfo> scopeInfoOpt = ctx.get(name);

    if(!scopeInfoOpt) {
        m_message.unknownVariable(name);
        return nullptr;
    }

    std::unique_ptr<VariableAccessNode> variableAccess = std::make_unique<VariableAccessNode>();
    variableAccess->m_stackIndex = scopeInfoOpt->m_stackIndex;
    variableAccess->m_type = scopeInfoOpt->m_type;
    variableAccess->m_size = scopeInfoOpt->getSize();
    variableAccess->m_memoryType = ValueType::LVALUE;
    
    return variableAccess;
}