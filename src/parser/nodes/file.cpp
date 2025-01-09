#include "file.hpp"

std::unordered_map<std::string, TypeInfo> FileNode::s_typeTable = {
    {"void", TypeInfo{"void", 0}},

    {"int", TypeInfo{"int", 4, {
        {"+", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<int>() + rhs.get<int>());
        }, "int"}
        },
        {"-", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<int>() - rhs.get<int>());
        }, "int"}
        },
        {"*", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<int>() * rhs.get<int>());
        }, "int"}
        },
        {"/", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<int>() / rhs.get<int>());
        }, "int"}
        },
        {"==", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<int>() == rhs.get<int>());
        }, "bool"}
        },
        {">", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<int>() > rhs.get<int>());
        }, "bool"}
        },
        {">=", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<int>() >= rhs.get<int>());
        }, "bool"}
        },
        {"<", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<int>() < rhs.get<int>());
        }, "bool"}
        },
        {"<=", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<int>() <= rhs.get<int>());
        }, "bool"}
        }
    }}},

    {"float", TypeInfo{"float", 4, {
        {"+", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<float>() + rhs.get<float>());
        }, "float"}
        },
        {"-", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<float>() - rhs.get<float>());
        }, "float"}
        },
        {"*", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<float>() * rhs.get<float>());
        }, "float"}
        },
        {"/", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<float>() / rhs.get<float>());
        }, "float"}
        },
        {"==", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<float>() == rhs.get<float>());
        }, "bool"}
        },
        {">", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<float>() > rhs.get<float>());
        }, "bool"}
        },
        {">=", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<float>() >= rhs.get<float>());
        }, "bool"}
        },
        {"<", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<float>() < rhs.get<float>());
        }, "bool"}
        },
        {"<=", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<float>() <= rhs.get<float>());
        }, "bool"}
        }
    }}},

    {"double", TypeInfo{"double", 8, {
        {"+", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<double>() + rhs.get<double>());
        }, "double"}
        },
        {"-", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<double>() - rhs.get<double>());
        }, "double"}
        },
        {"*", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<double>() * rhs.get<double>());
        }, "double"}
        },
        {"/", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<double>() / rhs.get<double>());
        }, "double"}
        },
        {"==", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<double>() == rhs.get<double>());
        }, "bool"}
        },
        {">", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<double>() > rhs.get<double>());
        }, "bool"}
        },
        {">=", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<double>() >= rhs.get<double>());
        }, "bool"}
        },
        {"<", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<double>() < rhs.get<double>());
        }, "bool"}
        },
        {"<=", {[](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<double>() <= rhs.get<double>());
        }, "bool"}
        }
    }}},

    {"bool", TypeInfo{"bool", 1}},

    {"char", TypeInfo{"char", 1}}
};


std::unordered_map<std::string, std::vector<std::shared_ptr<FunctionDefinitionNode>>> FileNode::s_functionTable = {
    {
        "print",
        {
            std::shared_ptr<FunctionDefinitionNode>(new NativeFunctionNode("print", {TypeInfo{"int", 4}}, TypeInfo{"void", 0}, &NativeFunctions::print)),
            std::shared_ptr<FunctionDefinitionNode>(new NativeFunctionNode("print", {TypeInfo{"float", 4}}, TypeInfo{"void", 0}, &NativeFunctions::print)),
            std::shared_ptr<FunctionDefinitionNode>(new NativeFunctionNode("print", {TypeInfo{"bool", 1}}, TypeInfo{"void", 0}, &NativeFunctions::print)),
            std::shared_ptr<FunctionDefinitionNode>(new NativeFunctionNode("print", {TypeInfo{"double", 8}}, TypeInfo{"void", 0}, &NativeFunctions::print))
        }
    }
};