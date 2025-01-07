#pragma once

#include "../../type_info.hpp"
#include "function_definition.hpp"
#include "native_function.hpp"
#include "../native_functions.hpp"
#include "evaluatable.hpp"
#include "../rules.hpp"

#include <unordered_map>
#include <optional>
#include <vector>
#include <memory>
#include <string>

class FileNode {
public:
    std::optional<TypeInfo> getTypeInfo(const std::string& type) {
        auto it = s_typeTable.find(type);
        if(it != s_typeTable.end())
            return it->second;

        it = m_typeTable.find(type);
        if(it != m_typeTable.end())
            return it->second;

        for(auto& import : m_imports) {
            auto it = import->getTypeInfo(type);
            if(it != std::nullopt)
                return it;
        }

        return std::nullopt;
    }

    bool registerFunction(std::shared_ptr<FunctionDefinitionNode> function, const std::string& name) {
        auto check = [&](const std::unordered_map<std::string, std::vector<std::shared_ptr<FunctionDefinitionNode>>>& table) -> bool {
            bool found = true;

            for(auto func : m_functionTable[name]) {
                if(func->m_parameters.size() != function->m_parameters.size()) {
                    found = false;
                    continue;
                }

                for(int i = 0; i < func->m_parameters.size(); i++) {
                    if(static_cast<const TypeInfo&>(func->m_parameters[i]) != function->m_parameters[i]) {
                        found = false;
                        continue;
                    }
                }

                break;
            }

            return found;
        };
        
        if(s_functionTable.contains(name) && check(s_functionTable))
            return false;

        if(!m_functionTable.contains(name))
            m_functionTable[name] = std::vector<std::shared_ptr<FunctionDefinitionNode>>{};
        else if(check(m_functionTable))
            return false;

        m_functionTable[name].push_back(function);
        return true;
    }

    std::shared_ptr<FunctionDefinitionNode> getFunction(const std::string& name, const std::vector<std::unique_ptr<EvaluatableNode>>& parameters) {
        auto check = [&](std::shared_ptr<FunctionDefinitionNode> func) -> bool {
            if(func->m_parameters.size() != parameters.size())
                return false;

            for(int i = 0; i < func->m_parameters.size(); i++) {
                if(!(Rules::isPtr(func->m_parameters[i].m_name) && Rules::isPtr(parameters[i]->m_type.m_name)) && func->m_parameters[i] != parameters[i]->m_type)
                    return false;
            }
            
            return true;
        };

        if(s_functionTable.contains(name)) {
            for(std::shared_ptr<FunctionDefinitionNode> func : s_functionTable[name]) {
                if(check(func))
                    return func;
            }
        }
        
        if(m_functionTable.contains(name)) {
            for(std::shared_ptr<FunctionDefinitionNode> func : m_functionTable[name]) {
                if(check(func))
                    return func;
            }
        }

        for(auto& import : m_imports) {
            if(import->m_functionTable.contains(name)) {
                for(std::shared_ptr<FunctionDefinitionNode> func : import->m_functionTable[name]) {
                    if(check(func))
                        return func;
                }
            }
        }

        return nullptr;
    }

public:
    std::unordered_map<std::string, TypeInfo> m_typeTable;
    std::unordered_map<std::string, std::vector<std::shared_ptr<FunctionDefinitionNode>>> m_functionTable;
    std::vector<std::shared_ptr<FileNode>> m_imports;

public:
    static std::unordered_map<std::string, TypeInfo> s_typeTable;
    static std::unordered_map<std::string, std::vector<std::shared_ptr<FunctionDefinitionNode>>> s_functionTable;
};