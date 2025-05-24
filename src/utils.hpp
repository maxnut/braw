#pragma once

#include "parser/nodes/identifier.hpp"
#include "type_info.hpp"
#include "braw_context.hpp"
#include "parser/nodes/function_definition.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <optional>

namespace Utils {

    inline std::string functionSignatureString(const AST::FunctionSignature& signature) {
        std::string funcString = ((AST::IdentifierNode*)signature.m_name.get())->m_name + "(";
        for(int i = 0; i < signature.m_parameters.size(); i++) {
            auto& parameter = signature.m_parameters[i];
            std::string parameterName = ((AST::IdentifierNode*)parameter->m_name.get())->m_name;
            if(parameterName.size() > 0)
                funcString += parameterName + ": ";
            funcString += ((AST::IdentifierNode*)parameter->m_type.get())->m_name;
            
            if(i < signature.m_parameters.size() - 1)
                funcString += ", ";
        }
        funcString += ") -> " + ((AST::IdentifierNode*)signature.m_returnType.get())->m_name;
        return funcString;
    }

    inline std::string sanitizeLabel(const std::string& input) {
        std::string output;
        for (size_t i = 0; i < input.size(); ++i) {
            switch (input[i]) {
                case '*': output += "_star"; break;
                case '(': output += "_"; break;
                case ')': output += "_"; break;
                case '.': output += "_dot"; break;
                default: output += input[i]; break;
            }
        }
        return output;
    } 

    inline std::string functionSignatureString(const FunctionSignature& signature) {
        std::string funcString = signature.m_name + "_";
        for(int i = 0; i < signature.m_parameters.size(); i++) {
            auto& parameter = signature.m_parameters[i];
            funcString += parameter.m_name;
            
            if(i < signature.m_parameters.size() - 1)
                funcString += "_";
        }
        funcString += "_" + signature.m_returnType.m_name;
        return sanitizeLabel(funcString);
    }

    inline TypeInfo makePointer(const TypeInfo& base) {
        return TypeInfo{base.m_name + "*", 8, true};
    }

    inline std::optional<TypeInfo> getRawType(const TypeInfo& pointer, const BrawContext& ctx) {
        std::string raw = pointer.m_name;
        if(raw.find("*") != std::string::npos)
            raw = raw.substr(0, raw.size() - 1);

        if(raw.find("*") != std::string::npos)
            return TypeInfo{raw, 8};


        return ctx.getTypeInfo(raw);
    }

    inline std::filesystem::path getStdPath() {
        std::filesystem::path stdPath;
        const char* path = std::getenv("BRAW_STDLIB");
        stdPath = path ? path : std::filesystem::current_path() / "stdlib";
        return stdPath;
    }

    inline void extendToWhitespace(const std::string& line, int& col, int direction, int maxExtension) {
        int originalCol = col;
        while (maxExtension > 0) {
            int newCol = col + direction;
            if (newCol < 0 || newCol >= static_cast<int>(line.size()) || std::isspace(line[newCol])) {
                break;
            }
            col = newCol;
            --maxExtension;
        }
        if (col == originalCol) { // If no extension happened, allow at least one character
            col = std::clamp(originalCol + direction, 0, static_cast<int>(line.size()));
        }
    }

    inline std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\n\r");
        size_t end = str.find_last_not_of(" \t\n\r");

        return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
    }

    inline std::string extractRangeWithContext(const std::string& filePath, int startLine, int startCol, int endLine, int endCol, int tabs = 2, bool allowMarkers = true, int contextChars = 30) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return "Error: Cannot open file.";
        }

        std::vector<std::string> lines;
        std::string line;
        
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();

        if (startLine < 1 || endLine > static_cast<int>(lines.size()) || startLine > endLine) {
            return "Error: Invalid line range.";
        }

        std::ostringstream extractedText;

        if (startLine == endLine && allowMarkers) {
            // Single-line range with underlining
            std::string& targetLine = lines[startLine - 1];

            int adjustedStartCol = std::max(0, startCol - 1);
            int adjustedEndCol = std::min(static_cast<int>(targetLine.size()), endCol);

            // Extend start and end positions to whitespace (if possible)
            extendToWhitespace(targetLine, adjustedStartCol, -1, contextChars);
            extendToWhitespace(targetLine, adjustedEndCol, 1, contextChars);

            extractedText << targetLine.substr(adjustedStartCol, adjustedEndCol - adjustedStartCol) << "\n";

            // Generate ^^^ marker
            extractedText << std::string(tabs, '\t');
            extractedText << std::string(std::max(startCol - adjustedStartCol - 2, 0), ' ') // Leading spaces
                        << std::string(endCol - startCol, '^');

        } else {
            // Multi-line range with expanded context
            for (int i = startLine - 1; i < endLine; ++i) {
                std::string& currentLine = lines[i];

                int lineStartCol = (i == startLine - 1) ? std::max(0, startCol - 1) : 0;
                int lineEndCol = (i == endLine - 1) ? std::min(static_cast<int>(currentLine.size()), endCol) : static_cast<int>(currentLine.size());

                extendToWhitespace(currentLine, lineStartCol, -1, contextChars);
                extendToWhitespace(currentLine, lineEndCol, 1, contextChars);

                if (lineStartCol < static_cast<int>(currentLine.size())) {
                    extractedText << currentLine.substr(lineStartCol, lineEndCol - lineStartCol);
                }
            }
        }


        return std::string(tabs, '\t') + trim(extractedText.str());
    }

    inline std::string uniqueLabelName() {
        static size_t counter = 0;
        return "." + std::to_string(counter++);
    }

    inline std::string uniqueRegisterName() {
        static size_t counter = 0;
        return "%" + std::to_string(counter++);
    }

    inline std::string getIdentifier(std::shared_ptr<AST::Node> node) {
        if(node->m_type == AST::Node::Identifier)
            return ((AST::IdentifierNode*)node.get())->m_name;
        return "";
    }
}