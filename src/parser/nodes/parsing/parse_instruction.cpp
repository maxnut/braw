#include "parser/parser.hpp"
#include "../variable_declaration.hpp"
#include "../binary_operator.hpp"
#include "../return.hpp"
#include "../if.hpp"
#include "../while.hpp"

Result<std::unique_ptr<AST::Node>> Parser::parseInstruction(TokenCursor& cursor, const std::filesystem::path& path) {
    Result<std::unique_ptr<AST::Node>> instruction;

    Rules::InstructionType instructionType = Rules::getInstructionType(cursor);
    
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
    else
        instruction = parseExpression(cursor, path);

    if(!instruction)
        return instruction;

    switch(instructionType) {
        default: {
            if(!expectTokenType(cursor.get().value(), Token::SEMICOLON))
                return unexpectedTokenExpectedType(cursor.value(), Token::SEMICOLON, path);
            cursor.tryNext();
            break;
        }
        case Rules::InstructionType::WHILE:
        case Rules::InstructionType::IF:
            break;
    }
    
    return instruction;
}