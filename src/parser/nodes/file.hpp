#pragma once

#include "../../type_info.hpp"
#include "function_definition.hpp"

#include <unordered_map>
#include <optional>
#include <vector>
#include <memory>
#include <string>

class FileNode {
public:
    std::optional<TypeInfo> getTypeInfo(const std::string& type) {
        auto it = m_typeTable.find(type);
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
        if(!m_functionTable.contains(name))
            m_functionTable[name] = std::vector<std::shared_ptr<FunctionDefinitionNode>>{};
        else {
            bool found = true;

            for(auto func : m_functionTable[name]) {
                if(func->m_parameters.size() != function->m_parameters.size()) {
                    found = false;
                    continue;
                }

                for(int i = 0; i < func->m_parameters.size(); i++) {
                    if(func->m_parameters[i] != function->m_parameters[i]) {
                        found = false;
                        continue;
                    }
                }

                break;
            }

            if(found) 
                return false;
        }

        m_functionTable[name].push_back(function);
        return true;
    }

public:
    std::unordered_map<std::string, TypeInfo> m_typeTable;
    std::unordered_map<std::string, std::vector<std::shared_ptr<FunctionDefinitionNode>>> m_functionTable;
    std::vector<std::shared_ptr<FileNode>> m_imports;
};