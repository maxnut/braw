#include "parser/parser.hpp"
#include "../variable_declaration.hpp"

std::unique_ptr<FunctionInstructionNode> Parser::parseVariableDeclaration(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    bool assignment = Rules::isAssignment(cursor);

    std::optional<TypeInfo> typeOpt = file->getTypeInfo(cursor.get().value().m_value);
    if(!typeOpt) {
        m_message.unknownType(cursor.value().m_value);
        return nullptr;
    }

    TypeInfo type = typeOpt.value();

    while(cursor.hasNext() && cursor.peekNext().m_value == "*") {
        type = makePointer(type);
        cursor.next();
    }

    ctx.m_scopeTables.front()[cursor.next().get().next().value().m_value] = ScopeInfo(type, ctx.m_currentStackSize, 0);
    ctx.changeStackSize(type.m_size);

    std::unique_ptr<VariableDeclarationNode> variableDeclaration = std::make_unique<VariableDeclarationNode>();
    variableDeclaration->m_type = type;
    variableDeclaration->size = type.m_size; //TODO array size

    if(assignment) {
        cursor.next();
        variableDeclaration->m_assignmentValue = parseExpression(file, cursor, ctx);

        if(!variableDeclaration->m_assignmentValue)
            return nullptr;

        if(variableDeclaration->m_type != variableDeclaration->m_assignmentValue->m_type) {
            m_message.mismatchedTypes(variableDeclaration->m_type.m_name, variableDeclaration->m_assignmentValue->m_type.m_name);
            return nullptr;
        }
    }

    return variableDeclaration;
}