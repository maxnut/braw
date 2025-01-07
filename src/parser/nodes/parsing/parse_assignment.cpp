#include "parser/parser.hpp"
#include "../assignment.hpp"

std::unique_ptr<FunctionInstructionNode> Parser::parseAssignment(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    std::unique_ptr<AssignmentNode> assignment = std::make_unique<AssignmentNode>();

    assignment->m_variable = parseExpression(file, cursor, ctx);
    if(!assignment->m_variable)
        return nullptr;

    if(assignment->m_variable->m_memoryType == ValueType::PRVALUE || assignment->m_variable->m_memoryType == ValueType::XVALUE) {
        m_message.unexpectedValueCategories(assignment->m_variable->m_memoryType, {ValueType::LVALUE, ValueType::XVALUE});
        return nullptr;
    }

    if(!expectTokenType(cursor.get().next().value(), Token::ASSIGNMENT))
        return nullptr;

    assignment->m_value = parseExpression(file, cursor, ctx);
    if(!assignment->m_value)
        return nullptr;

    return assignment;
}