#pragma once
#include <string>
#include <sstream>  

template<typename T>
std::string toString(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}