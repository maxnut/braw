#include "parser/parser.hpp"

std::unique_ptr<ScopeNode> Parser::parseScope(std::shared_ptr<FileNode> file, TokenCursor& cursor, ParserFunctionContext& ctx) {
    if(!expectTokenType(cursor.get().next().value(), Token::LEFT_BRACE))
        return nullptr;

    std::unique_ptr<ScopeNode> scope = std::make_unique<ScopeNode>();

    size_t initialStackSize = ctx.m_currentStackSize;
    ctx.m_scopeTables.push_front(std::unordered_map<std::string, ScopeInfo>());

    while(cursor.hasNext() && cursor.get().value().m_type != Token::RIGHT_BRACE) {
        std::unique_ptr<FunctionInstructionNode> instruction = parseInstruction(file, cursor, ctx);
        if(!instruction)
            return nullptr;
        scope->m_instructions.push_back(instruction);
    }

    ctx.m_scopeTables.pop_front();
    ctx.setStackSize(initialStackSize);

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_BRACE))
        return nullptr;

    return scope;
}