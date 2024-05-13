#include "CO2Sensor.hpp"

CO2Sensor::CO2Sensor(const std::string &port) : UARTPort(io, port)
{
    UARTPort.set_option(boost::asio::serial_port_base::baud_rate(9600));
    command[0] = 0xFF;
    command[1] = 0x01;
    command[5] = 0x00;
}

CO2Sensor::~CO2Sensor() { UARTPort.close(); }

int CO2Sensor::readCO2()
{

    command[2] = 0x86;
    command[3] = 0x00;
    command[4] = 0x00;
    command[6] = 0x00;
    command[7] = 0x00;

    unsigned char res[9];

    sendCommand();

    boost::asio::read(UARTPort, boost::asio::buffer(res, sizeof(res)));

    if (res[0] == 0xFF && res[1] == 0x86) {
        int co2 = (res[2] * 256) + res[3];
        return co2;
    }

    return -1;
}

void CO2Sensor::calibrateZero()
{
    command[2] = 0x87;
    command[3] = 0x00;
    command[4] = 0x00;
    command[6] = 0x00;
    command[7] = 0x00;

    sendCommand();
}

void CO2Sensor::calibrateSpan(int span)
{
    command[2] = 0x88;
    command[3] = static_cast<unsigned char>(span / 256);
    command[4] = static_cast<unsigned char>(span % 256);
    command[6] = 0x00;
    command[7] = 0x00;

    sendCommand();
}

void CO2Sensor::enableAutoCalibration()
{
    command[2] = 0x79;
    command[3] = 0xA0;
    command[4] = 0x00;
    command[6] = 0x00;
    command[7] = 0x00;

    sendCommand();
}

void CO2Sensor::disableAutoCalibration()
{
    command[2] = 0x79;
    command[3] = 0x00;
    command[4] = 0x00;
    command[6] = 0x00;
    command[7] = 0x00;

    sendCommand();
}

void CO2Sensor::setDetectionRange(int range)
{

    switch (range) {
    case 5000:
        command[6] = 0x13;
        command[7] = 0x88;
        break;
    case 2000:
        command[6] = 0x07;
        command[7] = 0xD0;
        break;
    default:
        return;
    }

    command[2] = 0x99;
    command[3] = 0x00;
    command[4] = 0x00;

    sendCommand();
}

void CO2Sensor::sendCommand()
{
    command[8] = getCheckSum();
    boost::asio::write(UARTPort, boost::asio::buffer(command, sizeof(command)));
}

char CO2Sensor::getCheckSum()
{
    unsigned char checksum;
    for (int i = 1; i < 8; i++) {
        checksum += command[i];
    }
    checksum = 0xFF - checksum;
    checksum += 1;
    return checksum;
}