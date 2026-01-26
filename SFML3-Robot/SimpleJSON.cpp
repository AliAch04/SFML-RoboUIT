#include "SimpleJSON.h"
#include <sstream>
#include <iostream>
#include <cctype>

std::string SimpleJSON::stringify(const std::vector<std::string>& maze,
    const std::string& name,
    int width, int height) {
    std::stringstream json;
    json << "{\n";
    json << "  \"name\": \"" << name << "\",\n";
    json << "  \"width\": " << width << ",\n";
    json << "  \"height\": " << height << ",\n";
    json << "  \"layout\": [\n";

    for (size_t i = 0; i < maze.size(); ++i) {
        json << "    \"" << maze[i] << "\"";
        if (i < maze.size() - 1) json << ",";
        json << "\n";
    }

    json << "  ]\n";
    json << "}";
    return json.str();
}

bool SimpleJSON::parse(const std::string& jsonStr, Object& obj) {
    obj.clear();
    size_t pos = 0;
    
    auto skipWhitespace = [&]() {
        while (pos < jsonStr.size() && std::isspace(jsonStr[pos])) pos++;
    };
    
    auto parseString = [&]() -> std::string {
        std::string result;
        pos++; // Skip opening quote
        while (pos < jsonStr.size() && jsonStr[pos] != '"') {
            if (jsonStr[pos] == '\\') pos++; // Skip escape
            if (pos < jsonStr.size()) result += jsonStr[pos];
            pos++;
        }
        if (pos < jsonStr.size() && jsonStr[pos] == '"') pos++;
        return result;
    };
    
    auto parseValue = [&]() -> std::string {
        skipWhitespace();
        if (pos >= jsonStr.size()) return "";
        
        if (jsonStr[pos] == '"') {
            return parseString();
        } else {
            // Parse number or boolean
            std::string result;
            while (pos < jsonStr.size() && jsonStr[pos] != ',' && 
                   jsonStr[pos] != '}' && jsonStr[pos] != ']' && !std::isspace(jsonStr[pos])) {
                result += jsonStr[pos];
                pos++;
            }
            // Remove trailing whitespace
            while (!result.empty() && std::isspace(result.back())) {
                result.pop_back();
            }
            return result;
        }
    };
    
    try {
        skipWhitespace();
        if (pos >= jsonStr.size() || jsonStr[pos] != '{') return false;
        pos++; // Skip '{'
        
        while (pos < jsonStr.size() && jsonStr[pos] != '}') {
            skipWhitespace();
            if (pos >= jsonStr.size()) return false;
            
            // Parse key
            if (jsonStr[pos] != '"') return false;
            std::string key = parseString();
            
            skipWhitespace();
            if (pos >= jsonStr.size() || jsonStr[pos] != ':') return false;
            pos++; // Skip ':'
            
            // Parse value
            std::string value = parseValue();
            obj[key] = value;
            
            skipWhitespace();
            if (pos < jsonStr.size() && jsonStr[pos] == ',') {
                pos++; // Skip ','
            }
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

int SimpleJSON::getInt(const Object& obj, const std::string& key, int defaultValue) {
    auto it = obj.find(key);
    if (it != obj.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

std::string SimpleJSON::getString(const Object& obj, const std::string& key,
    const std::string& defaultValue) {
    auto it = obj.find(key);
    if (it != obj.end()) {
        return it->second;
    }
    return defaultValue;
}