#ifndef CO2SENSOR_HPP
#define CO2SENSOR_HPP

#include <boost/asio.hpp>
#include <string>

class CO2Sensor {
public:
    CO2Sensor(const std::string &port);

    ~CO2Sensor();

    // Read CO2 concentration in parts per million (PPM) using UART interface.
    // Returning value is integer
    int readCO2();

    // Perform zero point calibration.
    // Ensure the sensor has been exposed to an environment
    // with a CO2 concentration of 400PPM for over 20 minutes.
    void calibrateZero();

    // Perform span calibration.
    // Perform zero point calibration before span calibration.
    // It's recommended to use a span of 2000PPM, but at least 1000PPM.
    void calibrateSpan(int span);

    // Enable automatic calibration logic (ABC) provided by the sensor.
    // ABC logic is typically enabled by default.
    void enableAutoCalibration();

    // Disable automatic calibration logic (ABC) provided by the sensor.
    void disableAutoCalibration();

    // Set the detection range of the sensor to either 2000 or 5000 PPM.
    void setDetectionRange(int range);

    // Calculate checksum to verify data integrity.
    char getCheckSum();

private:
    boost::asio::io_service io;

    boost::asio::serial_port UARTPort;

    unsigned char command[9];

    void sendCommand();
};

#endif