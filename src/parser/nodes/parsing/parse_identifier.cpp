#include "parser/nodes/identifier.hpp"
#include "parser/parser.hpp"
#include "parser/nodes/macro_parameter_reference.hpp"
#include <memory>


Result<std::shared_ptr<AST::Node>> Parser::parseIdentifier(TokenCursor& cursor, ParserContext& ctx) {
    if(Rules::isMacroCall(cursor)) {
        auto call = parseMacroCall(cursor, ctx);

        if(!call)
            return std::unexpected{call.error()};
        return call;
    }
    else if(Rules::isMacroParameterReference(cursor))
        return parseMacroParameter(cursor, ctx);
    auto node = std::make_shared<AST::IdentifierNode>(cursor.get().value().m_value);
    cursor.tryNext();
    return node;
}