#include "parser/nodes/identifier.hpp"
#include "parser/nodes/macro_parameter_reference.hpp"
#include "parser/parser.hpp"

Result<std::shared_ptr<AST::Node>> Parser::parseTypename(TokenCursor& cursor, ParserContext& ctx) {
    if(Rules::isMacroCall(cursor)) {
        auto call = parseMacroCall(cursor, ctx);

        if(!call)
            return std::unexpected{call.error()};
        return call;
    }
    else if(Rules::isMacroParameterReference(cursor))
        return parseMacroParameter(cursor, ctx);

    if(!expectTokenTypes(cursor.get().value(), {Token::IDENTIFIER, Token::KEYWORD}))
        return unexpectedTokenExpectedTypes(cursor.value(), {Token::IDENTIFIER, Token::KEYWORD}, ctx.m_path);

    std::string id = cursor.get().value().m_value;

    while(cursor.hasNext() && cursor.peekNext().m_value == "*") {
        id += "*";
        cursor.next();
    }

    cursor.next();

    return std::make_shared<AST::IdentifierNode>(id);
}