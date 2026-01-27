#pragma once
#include <string>
#include <vector>
#include <map>

class SimpleJSON {
public:
    using Object = std::map<std::string, std::string>;

    static std::string stringify(const std::vector<std::string>& maze,
        const std::string& name,
        int width, int height);

    static bool parse(const std::string& jsonStr, Object& obj);

    static int getInt(const Object& obj, const std::string& key, int defaultValue = 0);
    static std::string getString(const Object& obj, const std::string& key,
        const std::string& defaultValue = "");
};