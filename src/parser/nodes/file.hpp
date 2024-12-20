#pragma once

#include "../../type_info.hpp"

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
public:
    std::unordered_map<std::string, TypeInfo> m_typeTable;
    std::vector<std::shared_ptr<FileNode>> m_imports;
};