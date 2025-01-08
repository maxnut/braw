#include "file.hpp"

std::unordered_map<std::string, TypeInfo> FileNode::s_typeTable = {
    {"void", TypeInfo{"void", 0}},
    {"int", TypeInfo{"int", 4}}
};


std::unordered_map<std::string, std::vector<std::shared_ptr<FunctionDefinitionNode>>> FileNode::s_functionTable = {
    {"print",
        {
            std::shared_ptr<FunctionDefinitionNode>(new NativeFunctionNode("print", {TypeInfo{"int", 4}}, TypeInfo{"void", 0}, &NativeFunctions::print)),
            std::shared_ptr<FunctionDefinitionNode>(new NativeFunctionNode("print", {TypeInfo{"float", 4}}, TypeInfo{"void", 0}, &NativeFunctions::print)),
            std::shared_ptr<FunctionDefinitionNode>(new NativeFunctionNode("print", {TypeInfo{"double", 8}}, TypeInfo{"void", 0}, &NativeFunctions::print))
        }
    }
};