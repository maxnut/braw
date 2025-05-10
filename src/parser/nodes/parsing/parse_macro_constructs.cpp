#include "parser/nodes/macro_foreach.hpp"
#include "parser/nodes/macro_if.hpp"
#include "parser/nodes/macro_make_function.hpp"
#include "parser/nodes/macro_make_variable.hpp"
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
    macroForeach->m_varName = cursor.get().value().m_value;
    if(!expectTokenType(cursor.next().get().next().value(), Token::COLON))
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

Result<std::shared_ptr<AST::MacroMakeFunctionNode>> Parser::parseMacroMakeFunction(TokenCursor& cursor, ParserContext& ctx) {
    std::shared_ptr<AST::MacroMakeFunctionNode> macroMakeFunction = std::make_shared<AST::MacroMakeFunctionNode>();
    macroMakeFunction->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    cursor.next(2);
    if(!expectTokenType(cursor.get().value(), Token::LEFT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_PAREN, ctx.m_path);
    cursor.next();

    auto nameOpt = parseMacroParameter(cursor, ctx);
    if(!nameOpt)
        return std::unexpected{nameOpt.error()};
    macroMakeFunction->m_name = nameOpt.value();

    if(!expectTokenType(cursor.get().next().value(), Token::COMMA))
        return unexpectedTokenExpectedType(cursor.value(), Token::COMMA, ctx.m_path);

    auto typeOpt = parseMacroParameter(cursor, ctx);
    if(!typeOpt)
        return std::unexpected{typeOpt.error()};
    macroMakeFunction->m_returnType = typeOpt.value();

    if(!expectTokenType(cursor.get().next().value(), Token::COMMA))
        return unexpectedTokenExpectedType(cursor.value(), Token::COMMA, ctx.m_path);

    auto scopeOpt = parseScope(cursor, ctx);
    if(!scopeOpt)
        return std::unexpected{scopeOpt.error()};
    macroMakeFunction->m_parameterContainer = scopeOpt.value();

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN, ctx.m_path);

    scopeOpt = parseScope(cursor, ctx);
    if(!scopeOpt)
        return std::unexpected{scopeOpt.error()};
    macroMakeFunction->m_body = std::move(scopeOpt.value());
    
    macroMakeFunction->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    return macroMakeFunction;
}

Result<std::shared_ptr<AST::Node>> Parser::parseMacroMakeVariable(TokenCursor& cursor, ParserContext& ctx) {
    std::shared_ptr<AST::MacroMakeVariableNode> macroMakeVariable = std::make_shared<AST::MacroMakeVariableNode>();
    macroMakeVariable->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};
    cursor.next(2);
    if(!expectTokenType(cursor.get().value(), Token::LEFT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::LEFT_PAREN, ctx.m_path);
    cursor.next();

    auto nameOpt = parseMacroParameter(cursor, ctx);
    if(!nameOpt)
        return std::unexpected{nameOpt.error()};
    macroMakeVariable->m_name = nameOpt.value();

    if(!expectTokenType(cursor.get().next().value(), Token::COMMA))
        return unexpectedTokenExpectedType(cursor.value(), Token::COMMA, ctx.m_path);
    
    auto typeOpt = parseMacroParameter(cursor, ctx);
    if(!typeOpt)
        return std::unexpected{typeOpt.error()};
    macroMakeVariable->m_type = typeOpt.value();

    if(!expectTokenType(cursor.get().next().value(), Token::RIGHT_PAREN))
        return unexpectedTokenExpectedType(cursor.value(), Token::RIGHT_PAREN, ctx.m_path);

    macroMakeVariable->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    return macroMakeVariable;
}
