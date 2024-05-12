#ifndef DATA_HPP
#define DATA_HPP

#include <functional>
#include <string>
#include <vector>

struct CO2Sample {
    std::string datetime{}; // sqlite datetime format - 'YYYY-MM-DD hh:mm:ss'
    std::string CO2Level{};
};

struct RequestData {
    std::string cmd{};
    std::string param1{};
};

struct ResponseData {
    std::vector<CO2Sample> measurements{};
};

// callback for Application layer to send data back to client
using SendResponseCallback = std::function<void(ResponseData)>;

// callback for Network layer to process data from client
using DoTaskCallback = std::function<void(RequestData, SendResponseCallback)>;

#endif