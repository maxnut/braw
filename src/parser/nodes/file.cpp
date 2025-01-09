#include "file.hpp"

std::unordered_map<std::string, TypeInfo> FileNode::s_typeTable = {
    {"void", TypeInfo{"void", 0}},

    {"int", TypeInfo{"int", 4, {
        {"+", [](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<int>() + rhs.get<int>());
        }},
        {"-", [](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<int>() - rhs.get<int>());
        }},
        {"*", [](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<int>() * rhs.get<int>());
        }},
        {"/", [](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<int>() / rhs.get<int>());
        }}
    }}},

    {"float", TypeInfo{"float", 4, {
        {"+", [](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<float>() + rhs.get<float>());
        }},
        {"-", [](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<float>() - rhs.get<float>());
        }},
        {"*", [](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<float>() * rhs.get<float>());
        }},
        {"/", [](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<float>() / rhs.get<float>());
        }}
    }}},

    {"double", TypeInfo{"double", 8, {
        {"+", [](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<double>() + rhs.get<double>());
        }},
        {"-", [](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<double>() - rhs.get<double>());
        }},
        {"*", [](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<double>() * rhs.get<double>());
        }},
        {"/", [](Memory& dst, Memory& lhs, Memory& rhs) -> void {
            dst.from(lhs.get<double>() / rhs.get<double>());
        }}
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