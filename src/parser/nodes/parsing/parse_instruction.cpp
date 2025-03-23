#include "parser/parser.hpp"
#include "../variable_declaration.hpp"
#include "../binary_operator.hpp"
#include "../return.hpp"
#include "../if.hpp"
#include "../while.hpp"
#include "../for.hpp"
#include "rules.hpp"

Result<std::shared_ptr<AST::Node>> Parser::parseInstruction(TokenCursor& cursor, const std::filesystem::path& path, std::shared_ptr<AST::FileNode> file) {
    Result<std::shared_ptr<AST::Node>> instruction;

    if(Rules::isVariableDeclaration(cursor))
        instruction = parseVariableDeclaration(cursor, path, file);
    else if(Rules::isAssignment(cursor))
        instruction = parseAssignment(cursor, path, file);
    else if(Rules::isReturn(cursor))
        instruction = parseReturn(cursor, path, file);
    else if(Rules::isIf(cursor))
        instruction = parseIf(cursor, path, file);
    else if(Rules::isWhile(cursor))
        instruction = parseWhile(cursor, path, file);
    else if(Rules::isFor(cursor))
        instruction = parseFor(cursor, path, file);
    else
        instruction = parseExpression(cursor, path, file);
    
    return instruction;
}