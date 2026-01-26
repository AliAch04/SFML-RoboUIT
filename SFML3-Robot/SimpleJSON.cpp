#include "SimpleJSON.h"
#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include <stdexcept>

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

// Fonctions de parsing
void SimpleJSON::skipWhitespace(const std::string& json, size_t& pos) {
    while (pos < json.size() && std::isspace(json[pos])) {
        pos++;
    }
}

std::string SimpleJSON::parseString(const std::string& json, size_t& pos) {
    std::string result;
    pos++; // Skip opening quote

    while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\') {
            pos++;
            if (pos < json.size()) {
                switch (json[pos]) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case '/': result += '/'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                default: result += json[pos]; break;
                }
            }
        }
        else {
            result += json[pos];
        }
        pos++;
    }

    pos++; // Skip closing quote
    return result;
}

int SimpleJSON::parseInt(const std::string& json, size_t& pos) {
    std::string numStr;

    while (pos < json.size() && (std::isdigit(json[pos]) || json[pos] == '-')) {
        numStr += json[pos];
        pos++;
    }

    return std::stoi(numStr);
}

SimpleJSON::Value SimpleJSON::parseValue(const std::string& json, size_t& pos) {
    skipWhitespace(json, pos);

    if (pos >= json.size()) {
        return nullptr;
    }

    char c = json[pos];

    if (c == '"') {
        // String
        return parseString(json, pos);
    }
    else if (c == '-' || std::isdigit(c)) {
        // Number
        return parseInt(json, pos);
    }
    else if (c == 't' && json.substr(pos, 4) == "true") {
        pos += 4;
        return true;
    }
    else if (c == 'f' && json.substr(pos, 5) == "false") {
        pos += 5;
        return false;
    }
    else if (c == 'n' && json.substr(pos, 4) == "null") {
        pos += 4;
        return nullptr;
    }
    else if (c == '{') {
        // Object
        Object obj;
        pos++; // Skip '{'
        skipWhitespace(json, pos);

        while (pos < json.size() && json[pos] != '}') {
            // Parse key
            skipWhitespace(json, pos);
            if (json[pos] != '"') {
                throw std::runtime_error("Expected string key");
            }
            std::string key = parseString(json, pos);

            // Skip colon
            skipWhitespace(json, pos);
            if (json[pos] != ':') {
                throw std::runtime_error("Expected ':'");
            }
            pos++;

            // Parse value
            skipWhitespace(json, pos);
            Value value = parseValue(json, pos);
            obj[key] = value;

            // Skip comma
            skipWhitespace(json, pos);
            if (json[pos] == ',') {
                pos++;
            }
        }

        pos++; // Skip '}'
        return obj;
    }
    else if (c == '[') {
        // Array
        Array arr;
        pos++; // Skip '['
        skipWhitespace(json, pos);

        while (pos < json.size() && json[pos] != ']') {
            Value value = parseValue(json, pos);
            arr.push_back(value);

            skipWhitespace(json, pos);
            if (json[pos] == ',') {
                pos++;
            }
        }

        pos++; // Skip ']'
        return arr;
    }

    return nullptr;
}

bool SimpleJSON::parse(const std::string& jsonStr, Object& obj) {
    try {
        size_t pos = 0;
        skipWhitespace(jsonStr, pos);

        if (pos >= jsonStr.size() || jsonStr[pos] != '{') {
            return false;
        }

        Value value = parseValue(jsonStr, pos);
        if (auto* o = std::get_if<Object>(&value)) {
            obj = *o;
            return true;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }

    return false;
}

int SimpleJSON::getInt(const Object& obj, const std::string& key, int defaultValue) {
    auto it = obj.find(key);
    if (it != obj.end()) {
        if (auto* i = std::get_if<int>(&it->second)) {
            return *i;
        }
        else if (auto* d = std::get_if<double>(&it->second)) {
            return static_cast<int>(*d);
        }
    }
    return defaultValue;
}

std::string SimpleJSON::getString(const Object& obj, const std::string& key,
    const std::string& defaultValue) {
    auto it = obj.find(key);
    if (it != obj.end()) {
        if (auto* s = std::get_if<std::string>(&it->second)) {
            return *s;
        }
    }
    return defaultValue;
}