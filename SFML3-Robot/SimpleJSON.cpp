#include "SimpleJSON.h"
#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include <stdexcept>
#include <algorithm> // Pour std::isdigit avec conversion de type

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
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos]))) {
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

    if (pos < json.size() && json[pos] == '"') {
        pos++; // Skip closing quote
    }
    return result;
}

int SimpleJSON::parseInt(const std::string& json, size_t& pos) {
    std::string numStr;

    // Avancer pour capturer le signe négatif si présent
    if (pos < json.size() && json[pos] == '-') {
        numStr += json[pos];
        pos++;
    }

    // Capturer les chiffres
    while (pos < json.size() && std::isdigit(static_cast<unsigned char>(json[pos]))) {
        numStr += json[pos];
        pos++;
    }

    if (numStr.empty() || (numStr == "-")) {
        throw std::runtime_error("Invalid integer");
    }

    return std::stoi(numStr);
}

double SimpleJSON::parseDouble(const std::string& json, size_t& pos) {
    std::string numStr;
    size_t startPos = pos;

    // Capturer le nombre complet
    while (pos < json.size() &&
        (std::isdigit(static_cast<unsigned char>(json[pos])) ||
            json[pos] == '-' ||
            json[pos] == '.' ||
            json[pos] == 'e' ||
            json[pos] == 'E' ||
            json[pos] == '+')) {
        numStr += json[pos];
        pos++;
    }

    if (numStr.empty()) {
        throw std::runtime_error("Invalid double");
    }

    return std::stod(numStr);
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
    else if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) {
        // Number - essayer de parser comme double d'abord
        size_t savedPos = pos;

        try {
            // Essayer de parser comme double
            double d = parseDouble(json, pos);

            // Vérifier si c'était un entier (pas de . e E)
            std::string numStr = json.substr(savedPos, pos - savedPos);
            bool hasDecimalOrExponent = (numStr.find('.') != std::string::npos ||
                numStr.find('e') != std::string::npos ||
                numStr.find('E') != std::string::npos);

            if (!hasDecimalOrExponent) {
                // C'était un entier, remettre et parser comme int
                pos = savedPos;
                return parseInt(json, pos);
            }

            // C'était un double, on a déjà la valeur
            return d;
        }
        catch (...) {
            // Si double échoue, essayer int
            pos = savedPos;
            return parseInt(json, pos);
        }
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
                skipWhitespace(json, pos);
            }
        }

        if (pos < json.size() && json[pos] == '}') {
            pos++; // Skip '}'
        }

        // CORRECTION ICI : Retourner l'Object wrapper dans Value
        return Value(obj);
    }
    else if (c == '[') {
        // Array
        Array arr;
        pos++; // Skip '['
        skipWhitespace(json, pos);

        while (pos < json.size() && json[pos] != ']') {
            skipWhitespace(json, pos);
            if (json[pos] == ']') break;

            Value value = parseValue(json, pos);
            arr.push_back(value);

            skipWhitespace(json, pos);
            if (json[pos] == ',') {
                pos++;
            }
        }

        if (pos < json.size() && json[pos] == ']') {
            pos++; // Skip ']'
        }

        // CORRECTION ICI : Retourner l'Array wrapper dans Value
        return Value(arr);
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

        // Essayer d'extraire l'Object du variant
        if (std::holds_alternative<Object>(value)) {
            obj = std::get<Object>(value);
            return true;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Unknown JSON parse error" << std::endl;
        return false;
    }

    return false;
}

int SimpleJSON::getInt(const Object& obj, const std::string& key, int defaultValue) {
    auto it = obj.find(key);
    if (it != obj.end()) {
        const Value& value = it->second;
        if (std::holds_alternative<int>(value)) {
            return std::get<int>(value);
        }
        else if (std::holds_alternative<double>(value)) {
            return static_cast<int>(std::get<double>(value));
        }
    }
    return defaultValue;
}

std::string SimpleJSON::getString(const Object& obj, const std::string& key,
    const std::string& defaultValue) {
    auto it = obj.find(key);
    if (it != obj.end()) {
        const Value& value = it->second;
        if (std::holds_alternative<std::string>(value)) {
            return std::get<std::string>(value);
        }
    }
    return defaultValue;
}

double SimpleJSON::getDouble(const Object& obj, const std::string& key, double defaultValue) {
    auto it = obj.find(key);
    if (it != obj.end()) {
        const Value& value = it->second;
        if (std::holds_alternative<double>(value)) {
            return std::get<double>(value);
        }
        else if (std::holds_alternative<int>(value)) {
            return static_cast<double>(std::get<int>(value));
        }
    }
    return defaultValue;
}

bool SimpleJSON::getBool(const Object& obj, const std::string& key, bool defaultValue) {
    auto it = obj.find(key);
    if (it != obj.end()) {
        const Value& value = it->second;
        if (std::holds_alternative<bool>(value)) {
            return std::get<bool>(value);
        }
    }
    return defaultValue;
}