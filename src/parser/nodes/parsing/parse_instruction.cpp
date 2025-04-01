#include "parser/parser.hpp"
#include "../variable_declaration.hpp"
#include "../binary_operator.hpp"
#include "../return.hpp"
#include "../if.hpp"
#include "../while.hpp"
#include "../for.hpp"
#include "rules.hpp"

Result<std::shared_ptr<AST::Node>> Parser::parseInstruction(TokenCursor& cursor, ParserContext& ctx) {
    Result<std::shared_ptr<AST::Node>> instruction;

    if(Rules::isVariableDeclaration(cursor))
        instruction = parseVariableDeclaration(cursor, ctx, false, true);
    else if(Rules::isAssignment(cursor))
        instruction = parseAssignment(cursor, ctx);
    else if(Rules::isReturn(cursor))
        instruction = parseReturn(cursor, ctx);
    else if(Rules::isIf(cursor))
        instruction = parseIf(cursor, ctx);
    else if(Rules::isWhile(cursor))
        instruction = parseWhile(cursor, ctx);
    else if(Rules::isFor(cursor))
        instruction = parseFor(cursor, ctx);
    else if(cursor.get().value().m_type == Token::LEFT_BRACE)
        instruction = parseScope(cursor, ctx);
    else if(Rules::isMacroIf(cursor))
        instruction = parseMacroIf(cursor, ctx);
    else if(Rules::isMacroForeach(cursor))
        instruction = parseMacroForeach(cursor, ctx);
    else if(Rules::isMacroMakeFunction(cursor))
        instruction = parseMacroMakeFunction(cursor, ctx);
    else if(Rules::isMacroMakeVariable(cursor))
        instruction = parseMacroMakeVariable(cursor, ctx);
    else
        instruction = parseExpression(cursor, ctx);
    
    return instruction;
}