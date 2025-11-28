#include "SimpleJSON.h"
#include <sstream>
#include <string>

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