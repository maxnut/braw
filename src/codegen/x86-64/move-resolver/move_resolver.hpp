#pragma once

#include "codegen/operand.hpp"
#include "codegen/x86-64/instruction.hpp"
#include <cstdint>
#include <memory>
#include <unordered_set>
#include <vector>

namespace CodeGen::x86_64 {

struct MoveNode {
    std::unique_ptr<MoveNode> m_next;
    std::shared_ptr<Operand> m_operand;
};

class CodeGenerator;
class FunctionContext;

class MoveResolver {
public:   
    static std::vector<Instruction> resolve(std::vector<Instruction> from, std::unordered_set<size_t> ignore, CodeGenerator& codegen, FunctionContext& ctx);
    static bool operandEquals(std::shared_ptr<Operand> op1, std::shared_ptr<Operand> op2);
    
private:
    static uint64_t countOperand(std::shared_ptr<Operand> op, const std::vector<Instruction>& moves);
    static std::unique_ptr<MoveNode> buildGraph(const std::vector<Instruction>& moves);
    static std::shared_ptr<Operand> findLoop(const MoveNode* root);
};

}
