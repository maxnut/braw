#include "parser/parser.hpp"
#include "../variable_declaration.hpp"
#include "messages.hpp"

std::unique_ptr<FunctionInstructionNode> Parser::parseVariableDeclaration(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    bool assignment = Rules::isAssignment(cursor);

    std::optional<TypeInfo> type = file->getTypeInfo(cursor.get().value().m_value);
    if(!type) {
        Message::Error::unknownType(cursor.value().m_value);
        return nullptr;
    }

    ctx.m_scopeTables.front()[cursor.next().get().value().m_value] = ScopeInfo(type.value(), ctx.m_currentStackSize, 0);

    std::unique_ptr<VariableDeclarationNode> variableDeclaration = std::make_unique<VariableDeclarationNode>();
    variableDeclaration->m_type = type.value();

    if(assignment)
        variableDeclaration->m_assignmentValue = parseExpression(file, cursor, ctx);

    return variableDeclaration;
}