#include "parser/parser.hpp"
#include "interpreter/interpreter.hpp"
#include "parser/binder/binder.hpp"

#include <spdlog/spdlog.h>

#include <iostream>

int main() {
    spdlog::set_pattern("[%^%l%$] %v");
    
    Parser parser = Parser("main.braw");
    std::shared_ptr<FileNode> file = parser.parse();

    if(!file) {
        spdlog::error("Failed to parse file");
        return 1;
    }

    auto main = file->getFunction("main", {});
    if(!main) {
        spdlog::error("Failed to find main function");
        return 1;
    }

    size_t stackId = std::hash<std::thread::id>{}(std::this_thread::get_id());
    Interpreter interpreter;
    interpreter.m_stacks[stackId] = std::make_unique<Stack>();
    interpreter.invokeFunction(main.get(), *interpreter.m_stacks[stackId], nullptr, 0);

    Binder::closeHandles();

    return 0;
}