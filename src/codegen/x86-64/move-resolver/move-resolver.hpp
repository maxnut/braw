#pragma once

#include "codegen/operand.hpp"
#include "codegen/x86-64/instruction.hpp"
#include <cstdint>
#include <memory>
#include <vector>

namespace CodeGen::x86_64 {

struct MoveNode {
    std::unique_ptr<MoveNode> m_next;
    std::shared_ptr<Operand> m_operand;
};

class MoveResolver {
public:   
    static std::vector<Instruction> resolve(std::vector<Instruction> from);

private:
    static uint64_t countOperand(std::shared_ptr<Operand> op, const std::vector<Instruction>& moves);
    static std::unique_ptr<MoveNode> buildGraph(const std::vector<Instruction>& moves);
    static std::shared_ptr<Operand> findLoop(std::unique_ptr<MoveNode> root);
};

}
