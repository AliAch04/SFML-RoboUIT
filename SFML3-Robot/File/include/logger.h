#pragma once
#include <string>
// Logger system for MazeRobotSimulation

class Logger {
public:
    static void info(const std::string& msg);
    static void warn(const std::string& msg);
    static void error(const std::string& msg);
};
