#include "parser/parser.hpp"
#include "../variable_declaration.hpp"
#include "../binary_operator.hpp"
#include "../return.hpp"
#include "../if.hpp"
#include "../while.hpp"

Result<std::unique_ptr<AST::Node>> Parser::parseInstruction(TokenCursor& cursor) {
    Result<std::unique_ptr<AST::Node>> instruction;

    Rules::InstructionType instructionType = Rules::getInstructionType(cursor);
    
    if(Rules::isVariableDeclaration(cursor))
        instruction = parseVariableDeclaration(cursor);
    else if(Rules::isAssignment(cursor))
        instruction = parseAssignment(cursor);
    else if(Rules::isReturn(cursor))
        instruction = parseReturn(cursor);
    else if(Rules::isIf(cursor))
        instruction = parseIf(cursor);
    else if(Rules::isWhile(cursor))
        instruction = parseWhile(cursor);
    else
        instruction = parseExpression(cursor);

    switch(instructionType) {
        default: {
            if(!expectTokenType(cursor.get().next().value(), Token::SEMICOLON))
                return unexpectedTokenExpectedType(cursor.value(), Token::SEMICOLON);
            break;
        }
        case Rules::InstructionType::WHILE:
        case Rules::InstructionType::IF:
            break;
    }
    
    return instruction;
}