#ifndef DATA_HPP
#define DATA_HPP

#include <functional>
#include <string>
#include <vector>

struct CO2Sample {
    std::string datetime; // sqlite datetime format - 'YYYY-MM-DD hh:mm:ss'
    std::string CO2Level;
};

struct Data {
    std::vector<CO2Sample> measurements;
};

using ProcessDataCallback = std::function<void(std::string, Data)>;

#endif