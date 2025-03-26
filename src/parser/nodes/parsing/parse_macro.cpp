#include "parser/identifier.hpp"
#include "parser/nodes/macro.hpp"
#include "parser/parser.hpp"
#include "rules.hpp"

Result<std::shared_ptr<AST::MacroNode>> Parser::parseMacro(TokenCursor& cursor, ParserContext& ctx) {
    std::shared_ptr<AST::MacroNode> ret = std::make_shared<AST::MacroNode>();
    ret->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    if(!expectTokenValue(cursor.get().next().value(), "define"))
        return unexpectedTokenExpectedValue(cursor.value(), "define", ctx.m_path);

    ret->m_name = cursor.get().next().value().m_value;

    if(cursor.get().value().m_type == Token::LEFT_PAREN) {
        cursor.tryNext();
        while(cursor.hasNext()) {
            Identifier param = cursor.get().next().value().m_value;
            ret->m_parameters.push_back(param);
            if(cursor.get().value().m_type == Token::RIGHT_PAREN)
                break;
            if(!expectTokenType(cursor.get().next().value(), Token::COMMA))
                return unexpectedTokenExpectedType(cursor.value(), Token::COMMA, ctx.m_path);
        }
        cursor.tryNext();
    }
    
    ctx.m_currentMacro = ret;
    auto optNode = parseScope(cursor, ctx, true);
    ctx.m_currentMacro = nullptr;
    if(!optNode)
        return std::unexpected{optNode.error()};

    ret->m_node = optNode.value();
    ret->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    return ret;
}