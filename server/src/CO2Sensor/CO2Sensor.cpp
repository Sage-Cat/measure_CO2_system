#include "CO2Sensor.hpp"

CO2Sensor::CO2Sensor(const std::string &port) : io(), UARTPort(io, port)
{
    UARTPort.set_option(boost::asio::serial_port_base::baud_rate(9600));
}

CO2Sensor::~CO2Sensor() { UARTPort.close(); }

int CO2Sensor::readCO2()
{
    unsigned char cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
    unsigned char res[9];
    boost::asio::write(UARTPort, boost::asio::buffer(cmd, 9));
    boost::asio::read(UARTPort, boost::asio::buffer(res, 9));

    if (res[0] == 0xFF && res[1] == 0x86) {
        int co2 = (res[2] * 256) + res[3];
        return co2;
    }
    return -1;
}

void CO2Sensor::calibrateZero() {}

void CO2Sensor::calibrateSpan() {}

void CO2Sensor::onAutoCalibration() {}

void CO2Sensor::offAutoCalibration() {}

void CO2Sensor::setDetectionRange() {}

char CO2Sensor::getCheckSum() { return '0'; }