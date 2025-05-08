
#include "parser/nodes/macro_parameter_reference.hpp"
#include "parser/parser.hpp"

Result<std::shared_ptr<AST::MacroParameterReferenceNode>> Parser::parseMacroParameterReference(TokenCursor& cursor, ParserContext& ctx) {
    std::shared_ptr<AST::MacroParameterReferenceNode> ret = std::make_shared<AST::MacroParameterReferenceNode>();
    ret->m_rangeBegin = {cursor.get().value().m_line, cursor.get().value().m_column};

    if(!expectTokenValue(cursor.get().next().value(), "#"))
        return unexpectedTokenExpectedValue(cursor.value(), "#", ctx.m_path);

    if(!ctx.m_currentMacro)
        return notMacro(cursor.value(), ctx.m_path);

    ret->m_name = cursor.get().value().m_value;

    cursor.tryNext();
    ret->m_rangeEnd = {cursor.get().value().m_line, cursor.get().value().m_column};
    return ret;
}