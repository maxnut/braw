#include "compiler.hpp"
#include "ir/graph-color/graph_color.hpp"
#include "ir/instructions/call.hpp"
#include "cursor.hpp"

#include <array>
#include <memory>
#include <string>

bool rangeOverlaps(Range r, int i) {
    return r.m_range.first < i && i < r.m_range.second;
}

void Compiler::compile(const CallInstruction* instr, CompilerContext& ctx, std::ofstream& fs) {
    std::array<std::string, 17> callerSaved = {"rax","rcx","rdx","rsi","rdi","r8","r9","r10","r11","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"};

    std::vector<std::string> saveStack;

    for(auto& r : ctx.m_reg.m_ranges) {
        if(!rangeOverlaps(r.second, ctx.m_instructionIndex))
            continue;

        std::string mappedRegister = ctx.m_reg.m_registers.at(r.first); //TODO: spills

        if(std::find(callerSaved.begin(), callerSaved.end(), mappedRegister) == callerSaved.end())
            continue;

        saveStack.push_back(mappedRegister);
        fs << "push " << mappedRegister << "\n";
    }

    std::array<std::string, 6> parameterRegisters = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    Cursor<std::array<std::string, 6>::iterator> cursor(parameterRegisters.begin(), parameterRegisters.end());
    
    for(const Operand& p : instr->m_parameters) {
        switch(p.index()) {
            case 1: {
                //TODO: handle float registers (xmm0 etc...)
                //TODO: handle spills
                auto r = std::get<std::shared_ptr<Register>>(p);
                fs << "mov " << cursor.get().next().value() << ", " << ctx.m_reg.m_registers.at(r->m_id) << "\n";
                break;
            }
            case 2: {
                //TODO: handle constants other than int and bool (passing pointer to data)
                Value v = std::get<Value>(p);
                switch(v.index()) {
                    case 0:
                        fs << "mov " << cursor.get().next().value() << ", " << std::to_string(std::get<int>(v)) << "\n";
                        break;
                    case 4:
                        fs << "mov " << cursor.get().next().value() << ", " << std::to_string(std::get<bool>(v)) << "\n";
                        break;
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }
    }

    fs << "call " << instr->m_id << "\n";

    for(const std::string& save : saveStack)
        fs << "pop " << save << "\n";
}