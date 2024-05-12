#ifndef DATA_HPP
#define DATA_HPP

#include <functional>
#include <string>

struct Data {
    std::string currentCO2Level;
};

using ProcessDataCallback = std::function<void(std::string, Data)>;

#endif