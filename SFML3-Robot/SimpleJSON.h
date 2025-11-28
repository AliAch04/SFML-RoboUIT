#pragma once
#include <string>
#include <vector>

class SimpleJSON {
public:
    static std::string stringify(const std::vector<std::string>& maze,
        const std::string& name,
        int width, int height);
};