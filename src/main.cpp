#include "lexer/lexer.hpp"

#include <iostream>

int main() {
    std::optional<std::vector<Token>> tokens = Lexer(std::filesystem::path("main.braw")).tokenize();

    if(!tokens)
        return 1;

    

    return 0;
}