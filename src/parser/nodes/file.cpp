#include "file.hpp"

#define DIRECT_CAST(_from, _to) \
    {#_to, {[](Memory& from, Stack& stack) -> Memory { \
        Memory ret = stack.push(sizeof(_to)); \
        ret.from(static_cast<_to>(from.get<_from>())); \
        return ret; \
    }}} \

std::unordered_map<std::string, TypeInfo> FileNode::s_typeTable = {
    {"void", TypeInfo{"void", 0}},

    {"int", TypeInfo{"int", 4, 
        {
            {"+", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<int>() + rhs.get<int>());
            }, "int"}},
            {"-", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<int>() - rhs.get<int>());
            }, "int"}},
            {"*", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<int>() * rhs.get<int>());
            }, "int"} },
            {"/", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<int>() / rhs.get<int>());
            }, "int"}},
            {"==", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<int>() == rhs.get<int>());
            }, "bool"}},
            {"!=", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<int>() != rhs.get<int>());
            }, "bool"}},
            {">", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<int>() > rhs.get<int>());
            }, "bool"}},
            {">=", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<int>() >= rhs.get<int>());
            }, "bool"}},
            {"<", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<int>() < rhs.get<int>());
            }, "bool"}},
            {"<=", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<int>() <= rhs.get<int>());
            }, "bool"}}
        },
        {
            DIRECT_CAST(int, float),
            DIRECT_CAST(int, double),
            {"bool", {[](Memory& from, Stack& stack) -> Memory {
                Memory ret = stack.push(sizeof(bool));
                ret.from(from.get<int>() != 0);
                return ret;
            }}}
        }
    }},

    {"float", TypeInfo{"float", 4, 
        {
            {"+", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<float>() + rhs.get<float>());
            }, "float"}},
            {"-", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<float>() - rhs.get<float>());
            }, "float"}},
            {"*", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<float>() * rhs.get<float>());
            }, "float"}},
            {"/", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<float>() / rhs.get<float>());
            }, "float"}},
            {"==", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<float>() == rhs.get<float>());
            }, "bool"}},
            {"!=", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<float>() != rhs.get<float>());
            }, "bool"}},
            {">", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<float>() > rhs.get<float>());
            }, "bool"}},
            {">=", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<float>() >= rhs.get<float>());
            }, "bool"}},
            {"<", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<float>() < rhs.get<float>());
            }, "bool"}},
            {"<=", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<float>() <= rhs.get<float>());
            }, "bool"}}
        },
        {
            DIRECT_CAST(float, int),
            DIRECT_CAST(float, double),
            {"bool", {[](Memory& from, Stack& stack) -> Memory {
                Memory ret = stack.push(sizeof(bool));
                ret.from(from.get<float>() != 0);
                return ret;
            }}}
        }
    }},

    {"double", TypeInfo{"double", 8, 
        {
            {"+", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<double>() + rhs.get<double>());
            }, "double"}},
            {"-", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<double>() - rhs.get<double>());
            }, "double"}},
            {"*", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<double>() * rhs.get<double>());
            }, "double"}},
            {"/", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<double>() / rhs.get<double>());
            }, "double"}},
            {"==", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<double>() == rhs.get<double>());
            }, "bool"}},
            {"!=", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<double>() != rhs.get<double>());
            }, "bool"}},
            {">", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<double>() > rhs.get<double>());
            }, "bool"}},
            {">=", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<double>() >= rhs.get<double>());
            }, "bool"}},
            {"<", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<double>() < rhs.get<double>());
            }, "bool"}},
            {"<=", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
                dst.from(lhs.get<double>() <= rhs.get<double>());
            }, "bool"}}
        },
        {
            DIRECT_CAST(double, int),
            DIRECT_CAST(double, float),
            {"bool", {[](Memory& from, Stack& stack) -> Memory {
                Memory ret = stack.push(sizeof(bool));
                ret.from(from.get<double>() != 0);
                return ret;
            }}}
        }
    }},

    {"bool", TypeInfo{"bool", 1}},

    {"char", TypeInfo{"char", 1}}
};


std::unordered_map<std::string, std::vector<std::shared_ptr<FunctionDefinitionNode>>> FileNode::s_functionTable = {
    {
        "print",
        {
            std::shared_ptr<FunctionDefinitionNode>(new NativeFunctionNode("print", {TypeInfo{"int", 4}}, TypeInfo{"void", 0}, &NativeFunctions::print<int>)),
            std::shared_ptr<FunctionDefinitionNode>(new NativeFunctionNode("print", {TypeInfo{"float", 4}}, TypeInfo{"void", 0}, &NativeFunctions::print<float>)),
            std::shared_ptr<FunctionDefinitionNode>(new NativeFunctionNode("print", {TypeInfo{"bool", 1}}, TypeInfo{"void", 0}, &NativeFunctions::print<bool>)),
            std::shared_ptr<FunctionDefinitionNode>(new NativeFunctionNode("print", {TypeInfo{"char*", 8}}, TypeInfo{"void", 0}, &NativeFunctions::print<char*>)),
            std::shared_ptr<FunctionDefinitionNode>(new NativeFunctionNode("print", {TypeInfo{"double", 8}}, TypeInfo{"void", 0}, &NativeFunctions::print<double>))
        }
    },
    {
        "scan",
        {
            std::shared_ptr<FunctionDefinitionNode>(new NativeFunctionNode("scan", {TypeInfo{"int", 4}, TypeInfo{"char*", 8}}, TypeInfo{"void", 0}, &NativeFunctions::scan))
        }
    },
    {
        "memory_allocate",
        {
            std::shared_ptr<FunctionDefinitionNode>(new NativeFunctionNode("memory_allocate", {TypeInfo{"int", 4}}, TypeInfo{"void*", 8}, &NativeFunctions::memory_allocate))
        }
    },
    {
        "memory_free",
        {
            std::shared_ptr<FunctionDefinitionNode>(new NativeFunctionNode("memory_free", {TypeInfo{"void*", 8}}, TypeInfo{"void", 0}, &NativeFunctions::memory_free))
        }
    },
    {
        "memory_set",
        {
            std::shared_ptr<FunctionDefinitionNode>(new NativeFunctionNode("memory_set", {TypeInfo{"void*", 8}, TypeInfo{"int", 4}, TypeInfo{"int", 4}}, TypeInfo{"void", 0}, &NativeFunctions::memory_set))
        }
    }
};