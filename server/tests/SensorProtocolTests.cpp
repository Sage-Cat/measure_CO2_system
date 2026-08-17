#include "Devices/CO2Sensor.hpp"

#include <array>
#include <stdexcept>

int main()
{
    std::array<unsigned char, 9> response{0xFF, 0x86, 0x01, 0xF4, 0, 0, 0, 0, 0x85};
    if (!CO2Sensor::isValidReadResponse(response)) {
        throw std::runtime_error("valid MH-Z19B response was rejected");
    }
    response[8] ^= 0x01;
    if (CO2Sensor::isValidReadResponse(response)) {
        throw std::runtime_error("invalid MH-Z19B checksum was accepted");
    }
}
