#include "parser/nodes/macro_foreach.hpp"
#include "parser/nodes/macro_if.hpp"
#include "parser/parser.hpp"
#include <memory>

Result<std::shared_ptr<AST::Node>> Parser::parseMacroIf(TokenCursor& cursor, ParserContext& ctx) {
    std::shared_ptr<AST::MacroIfNode> macroIf = std::make_shared<AST::MacroIfNode>();
    macroIf->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    cursor.next(2);
    if(!expectTokenType(cursor.get().value(), Token::LEFT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_PAREN, ctx.m_path);
    cursor.next();
    auto callOpt = parseMacroCall(cursor, ctx);
    if(!callOpt)
        return std::unexpected{callOpt.error()};
    macroIf->m_condition = std::move(std::static_pointer_cast<AST::MacroCallNode>(callOpt.value()));
    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN, ctx.m_path);

    auto scopeOpt = parseScope(cursor, ctx);
    if(!scopeOpt)
        return std::unexpected{scopeOpt.error()};
    macroIf->m_then = std::move(scopeOpt.value());

    macroIf->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    return macroIf;
}

Result<std::shared_ptr<AST::Node>> Parser::parseMacroForeach(TokenCursor& cursor, ParserContext& ctx) {
    std::shared_ptr<AST::MacroForeachNode> macroForeach = std::make_shared<AST::MacroForeachNode>();
    macroForeach->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    cursor.next(2);
    if(!expectTokenType(cursor.get().value(), Token::LEFT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_PAREN, ctx.m_path);
    cursor.next();
    auto identifierOpt = parseIdentifier(cursor, ctx);
    if(!identifierOpt) return std::unexpected{identifierOpt.error()};
    macroForeach->m_varName = identifierOpt.value();
    if(!expectTokenType(cursor.get().next().value(), Token::COLON))
        return unexpectedTokenExpectedType(cursor.value(), Token::COLON, ctx.m_path);
    
    auto callOpt = parseMacroParameter(cursor, ctx);
    if(!callOpt)
        return std::unexpected{callOpt.error()};
    macroForeach->m_collection = std::move(callOpt.value());
    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN, ctx.m_path);

    auto scopeOpt = parseScope(cursor, ctx);
    if(!scopeOpt)
        return std::unexpected{scopeOpt.error()};
    macroForeach->m_body = std::move(scopeOpt.value());

    macroForeach->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    return macroForeach;
}
