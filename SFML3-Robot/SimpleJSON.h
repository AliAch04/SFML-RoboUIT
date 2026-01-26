#pragma once
#include <string>
#include <vector>
#include <map>
#include <any>
#include <variant>

class SimpleJSON {
public:
    using Value = std::variant<std::string, int, double, bool, std::nullptr_t>;
    using Object = std::map<std::string, Value>;
    using Array = std::vector<Value>;

    static std::string stringify(const std::vector<std::string>& maze,
        const std::string& name,
        int width, int height);

    // Nouvelles fonctionnalités pour parser les JSON
    static bool parse(const std::string& jsonStr, Object& obj);

    // Méthodes d'accès aux données
    static int getInt(const Object& obj, const std::string& key, int defaultValue = 0);
    static std::string getString(const Object& obj, const std::string& key,
        const std::string& defaultValue = "");

private:
    static Value parseValue(const std::string& json, size_t& pos);
    static std::string parseString(const std::string& json, size_t& pos);
    static int parseInt(const std::string& json, size_t& pos);
    static void skipWhitespace(const std::string& json, size_t& pos);
};