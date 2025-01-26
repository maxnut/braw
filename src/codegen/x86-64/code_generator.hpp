#pragma once

#include "../operand.hpp"
#include "ir/file.hpp"
#include "register.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <unordered_map>

namespace CodeGen::x86_64 {

struct FunctionContext {
    std::unordered_map<std::string, std::shared_ptr<Operand>> m_virtualRegisters;
};

class CodeGenerator {
public:
    CodeGenerator(const File& src);

private:
    void initializeRegisters();

private:
    std::unordered_map<std::string, std::shared_ptr<Operands::Register>> m_registers;
};
    
}