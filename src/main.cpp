#include "parser/parser.hpp"

#include <spdlog/spdlog.h>

#include <iostream>

int main() {
    spdlog::set_pattern("[%^%l%$] %v");
    
    Parser parser = Parser("main.braw");
    std::shared_ptr<FileNode> file = parser.parse();

    if(!file) {
        std::cout << "Failed to parse file" << std::endl;
        return 1;
    }

    return 0;
}