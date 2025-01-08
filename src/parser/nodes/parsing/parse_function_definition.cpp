#include "parser/parser.hpp"

std::shared_ptr<FunctionDefinitionNode> Parser::parseFunctionDefinition(std::shared_ptr<FileNode> file, TokenCursor& cursor) {
    std::shared_ptr<FunctionDefinitionNode> node = std::make_shared<FunctionDefinitionNode>();
    
    std::optional<TypeInfo> returnType = file->getTypeInfo(cursor.get().value().m_value);
    if(!returnType) {
        m_message.unknownType(cursor.value().m_value);
        return nullptr;
    }

    node->m_returnType = returnType.value();
    node->m_name = cursor.next().get().value().m_value;

    if(!expectTokenType(cursor.next().get().next().value(), Token::LEFT_PAREN))
        return nullptr;

    ParserFunctionContext ctx;
    std::unordered_map<std::string, ScopeInfo> scopeTable = std::unordered_map<std::string, ScopeInfo>();

    while(cursor.hasNext() && cursor.get().value().m_type != Token::RIGHT_PAREN) {
        std::optional<TypeInfo> parameter = file->getTypeInfo(cursor.get().value().m_value);
        if(!returnType) {
            m_message.unknownType(cursor.value().m_value);
            return nullptr;
        }
        if(!expectTokenType(cursor.next().get().value(), Token::IDENTIFIER))
            return nullptr;
        
        node->m_parameters.push_back(parameter.value());
        scopeTable[cursor.get().next().value().m_value] = ScopeInfo(parameter.value(), ctx.m_currentStackSize, 0);
        ctx.changeStackSize(parameter->m_size);
    }

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return nullptr;

    ctx.m_scopeTables.push_front(scopeTable);
    node->m_scope = parseScope(file, cursor, ctx);
    ctx.m_scopeTables.pop_front();

    if(!node->m_scope)
        return nullptr;

    return node;
}
