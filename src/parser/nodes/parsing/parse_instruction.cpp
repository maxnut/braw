#include "parser/parser.hpp"
#include "../variable_declaration.hpp"
#include "../binary_operator.hpp"
#include "../return.hpp"
#include "../if.hpp"
#include "../while.hpp"
#include "../for.hpp"
#include "rules.hpp"

Result<std::unique_ptr<AST::Node>> Parser::parseInstruction(TokenCursor& cursor, const std::filesystem::path& path) {
    Result<std::unique_ptr<AST::Node>> instruction;

    if(Rules::isVariableDeclaration(cursor))
        instruction = parseVariableDeclaration(cursor, path);
    else if(Rules::isAssignment(cursor))
        instruction = parseAssignment(cursor, path);
    else if(Rules::isReturn(cursor))
        instruction = parseReturn(cursor, path);
    else if(Rules::isIf(cursor))
        instruction = parseIf(cursor, path);
    else if(Rules::isWhile(cursor))
        instruction = parseWhile(cursor, path);
    else if(Rules::isFor(cursor))
        instruction = parseFor(cursor, path);
    else
        instruction = parseExpression(cursor, path);
    
    return instruction;
}