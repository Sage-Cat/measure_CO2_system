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
    std::vector<std::string> params{};
};

struct ResponseData {
    std::vector<CO2Sample> measurements{};
    std::string error{};
};

// Callback for Application layer to send data back to client
using SendResponseCallback = std::function<void(ResponseData)>;

// Callback for Network layer to process data from client
using DoTaskCallback = std::function<void(RequestData, SendResponseCallback)>;

// Callback for fetching outdoor CO2 from OpenWeatherMap
using FetchOutdoorCO2TaskCallback = std::function<std::string(const std::string &)>;

namespace JsonKeys {
inline constexpr auto CMD          = "cmd";
inline constexpr auto PARAMS       = "params";
inline constexpr auto ERR        = "error";
} // namespace JsonKeys

#endif // DATA_HPP
