#include "file.hpp"

std::unordered_map<std::string, TypeInfo> FileNode::s_typeTable = {
    {"void", TypeInfo{"void", 0}},
    {"int", TypeInfo{"int", 4}}
};

std::unordered_map<std::string, std::vector<std::shared_ptr<FunctionDefinitionNode>>> FileNode::s_functionTable = {
    {"print",
        {
            std::make_shared<NativeFunctionNode>(&NativeFunctions::print)
        }
    }
};