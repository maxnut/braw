#include "code_generator.hpp"
#include "codegen/x86-64/address.hpp"
#include "codegen/x86-64/register.hpp"
#include "ir/graph-color/graph_color.hpp"
#include <memory>

namespace CodeGen::x86_64 {

CodeGenerator::CodeGenerator(const File& src) {
    initializeRegisters();

    for(const Function& f : src.m_functions) {
        ColorResult result = GraphColor::build(f, {"rdi","rsi","rdx","rcx","r8","r9","rbx","r10","r11","r12","r13","r14","r15"}, {"xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"});
        FunctionContext ctx;

        for(Range* range : result.m_rangeVector) {
            size_t spills = 0;
            if(result.m_registers.contains(range->m_id)) {
                ctx.m_virtualRegisters[range->m_id] = m_registers.at(result.m_registers.at(range->m_id));
            }
            else {
                ctx.m_virtualRegisters[range->m_id] = std::make_shared<Operands::Address>(m_registers.at("rsp"), spills);
                spills += range->size();
            }
        }
    }
}

#define REGISTER(ID) m_registers[ID] = std::make_shared<Operands::Register>(ID)

void CodeGenerator::initializeRegisters() {
    REGISTER("rsp");
    REGISTER("rax");
    REGISTER("rdi");
    REGISTER("rsi");
    REGISTER("rdx");
    REGISTER("rcx");
    REGISTER("rbx");
    REGISTER("r8");
    REGISTER("r9");
    REGISTER("r10");
    REGISTER("r11");
    REGISTER("r12");
    REGISTER("r13");
    REGISTER("r14");
    REGISTER("r15");
    REGISTER("xmm0");
    REGISTER("xmm1");
    REGISTER("xmm2");
    REGISTER("xmm3");
    REGISTER("xmm4");
    REGISTER("xmm5");
    REGISTER("xmm6");
    REGISTER("xmm7");
    REGISTER("xmm8");
    REGISTER("xmm9");
    REGISTER("xmm10");
    REGISTER("xmm11");
    REGISTER("xmm12");
    REGISTER("xmm13");
    REGISTER("xmm14");
    REGISTER("xmm15");
}

}