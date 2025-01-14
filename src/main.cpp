#include "parser/parser.hpp"
#include "lexer/lexer.hpp"
#include "interpreter/interpreter.hpp"
#include "parser/binder/binder.hpp"

#include <spdlog/spdlog.h>

#include <iostream>

int main(int argc, char** argv) {
    spdlog::set_pattern("[%^%l%$] %v");
    spdlog::set_level(spdlog::level::debug);

    if(argc < 2) {
        spdlog::error("No file specified");
        return 1;
    }

    std::filesystem::path filepath(argv[1]);
    auto tokens = Lexer::tokenize(filepath);
    if(!tokens) {
        spdlog::error("Failed to tokenize file");
        return 1;
    }
    
    Parser p{};
    std::shared_ptr<FileNode> file = p.parseFile(filepath, tokens.value());

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