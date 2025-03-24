#include "parser/parser.hpp"
#include "../scope.hpp"

Result<std::shared_ptr<AST::ScopeNode>> Parser::parseScope(TokenCursor& cursor, ParserContext& ctx, bool allowOneLine) {
    std::shared_ptr<AST::ScopeNode> scope = std::make_shared<AST::ScopeNode>();
    scope->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};

    bool oneLine = cursor.get().value().m_type == Token::COLON && allowOneLine;
    if(oneLine)
        cursor.tryNext();

    if(!oneLine && !expectTokenType(cursor.get().next().value(), Token::LEFT_BRACE))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_BRACE, ctx.m_path);

    while(cursor.hasNext() && cursor.get().value().m_type != Token::RIGHT_BRACE) {
        Rules::InstructionType instructionType = Rules::getInstructionType(cursor);

        auto instructionOpt = parseInstruction(cursor, ctx);
        if(!instructionOpt)
            return std::unexpected{instructionOpt.error()};

        switch(instructionType) {
            default: {
                if(!expectTokenType(cursor.get().value(), Token::SEMICOLON))
                    return unexpectedTokenExpectedType(cursor.value(), Token::SEMICOLON, ctx.m_path);
                cursor.tryNext();
                break;
            }
            case Rules::InstructionType::WHILE:
            case Rules::InstructionType::SCOPE:
            case Rules::InstructionType::FOR:
            case Rules::InstructionType::IF:
                break;
        }
        
        scope->m_instructions.push_back(std::move(instructionOpt.value()));

        if(oneLine) break;
    }

    if(!oneLine && !expectTokenType(cursor.get().value(), Token::RIGHT_BRACE))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_BRACE, ctx.m_path);

    scope->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};

    if(!oneLine)
        cursor.tryNext();

    return scope;
}