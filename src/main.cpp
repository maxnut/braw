#include "parser/parser.hpp"

#include <iostream>

int main() {
    Parser parser = Parser("main.braw");
    std::shared_ptr<FileNode> file = parser.parse();

    if(!file) {
        std::cout << "Failed to parse file" << std::endl;
        return 1;
    }

    return 0;
}