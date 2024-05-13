#include "CO2Sensor.hpp"

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <stdexcept>

#include "SpdlogConfig.hpp"

CO2Sensor::CO2Sensor(const std::string &port)
    : uart_port_(io_, port), command_({0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00})
{
    SPDLOG_TRACE("CO2Sensor::CO2Sensor");
    // Set baud rate to 9600
    uart_port_.set_option(boost::asio::serial_port_base::baud_rate(9600));
}

CO2Sensor::~CO2Sensor()
{
    SPDLOG_TRACE("CO2Sensor::~CO2Sensor");
    // Close the serial port if it is open
    if (uart_port_.is_open()) {
        uart_port_.close();
    }
}

int CO2Sensor::readCO2()
{
    SPDLOG_TRACE("CO2Sensor::readCO2");
    // Command to read CO2 (mh-Z19B command).
    command_[2] = 0x86;

    sendCommand();

    std::array<unsigned char, 9> response{};
    boost::asio::read(uart_port_, boost::asio::buffer(response));

    // Response validation
    if (response[0] != 0xFF || response[1] != 0x86) {
        throw std::runtime_error("Invalid response from sensor.");
    }

    // Convert high and low bytes to integer.
    return (static_cast<int>(response[2]) << 8) + response[3];
}

void CO2Sensor::calibrateZero()
{
    SPDLOG_TRACE("CO2Sensor::calibrateZero");
    // Command to calibrate to 400 ppm (mh-Z19B zero calibration command).
    command_[2] = 0x87;
    sendCommand();
}

void CO2Sensor::calibrateSpan(int span)
{
    SPDLOG_TRACE("CO2Sensor::calibrateSpan - {}", span);
    // Validate span to ensure it's within sensible limits.
    if (span < MIN_SPAN_PPM) {
        throw std::invalid_argument("Span must be greater than or equal to MIN_SPAN_PPM.");
    }

    // Command for span calibration (mh-Z19B span calibration command).
    command_[2] = 0x88;
    command_[3] = static_cast<unsigned char>(span >> 8);
    command_[4] = static_cast<unsigned char>(span & 0xFF);

    sendCommand();
}

void CO2Sensor::enableAutoCalibration(bool enable)
{
    SPDLOG_TRACE("CO2Sensor::enableAutoCalibration - {}", enable);
    command_[2] = 0x79;                 // Command to toggle auto-calibration (mh-Z19B ABC command).
    command_[3] = enable ? 0xA0 : 0x00; // Enable or disable auto-calibration.

    sendCommand();
}

void CO2Sensor::setDetectionRange(Calibration range)
{
    SPDLOG_TRACE("CO2Sensor::setDetectionRange - {}", range == Calibration::Span2000 ? "2000" : "5000");
    constexpr std::array<std::pair<unsigned char, unsigned char>, 2> ranges = {{
        {0x07, 0xD0}, // Settings for 2000 ppm range.
        {0x13, 0x88}  // Settings for 5000 ppm range.
    }};

    // Command to set detection range (mh-Z19B range command).
    command_[2]      = 0x99;
    auto [high, low] = ranges[static_cast<int>(range) == 5000 ? 1 : 0];
    command_[6]      = high;
    command_[7]      = low;
    sendCommand();
}

void CO2Sensor::sendCommand()
{
    SPDLOG_TRACE("CO2Sensor::sendCommand");
    // Calculate and append checksum before sending command.
    command_[8] = calculateChecksum();
    boost::asio::write(uart_port_, boost::asio::buffer(command_));
}

char CO2Sensor::calculateChecksum() const
{
    SPDLOG_TRACE("CO2Sensor::calculateChecksum");
    unsigned char checksum = 0;
    for (size_t i = 1; i < command_.size() - 1; ++i) {
        checksum += command_[i];
    }

    // Final checksum calculation based on command contents.
    return 0xFF - checksum + 1;
}
