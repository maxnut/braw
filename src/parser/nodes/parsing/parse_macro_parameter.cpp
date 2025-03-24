
#include "parser/identifier.hpp"
#include "parser/nodes/macro_parameter.hpp"
#include "parser/parser.hpp"

Result<std::shared_ptr<AST::MacroParameterNode>> Parser::parseMacroParameter(TokenCursor& cursor, ParserContext& ctx) {
    std::shared_ptr<AST::MacroParameterNode> ret = std::make_shared<AST::MacroParameterNode>();
    ret->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};

    if(!expectTokenValue(cursor.get().next().value(), "#"))
        return unexpectedTokenExpectedValue(cursor.value(), "#", ctx.m_path);

    if(!ctx.m_currentMacro)
        return notMacro(cursor.value(), ctx.m_path);

    Identifier name = cursor.get().value().m_value;
    if(!ctx.m_currentMacro->m_parameters.contains(name))
        return unknownMacroParameter(cursor.value(), ctx.m_path);
    ret->m_index = ctx.m_currentMacro->m_parameters[name];

    ret->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    cursor.tryNext();
    return ret;
}