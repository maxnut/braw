#include "compiler.hpp"
#include "ir/function.hpp"

void Compiler::compile(const File& file, std::filesystem::path out) {
    std::ofstream fs(out);

    fs << "section .text\n";
    for(const Function& f : file.m_functions)
        compile(f, fs);

    fs.close();
}