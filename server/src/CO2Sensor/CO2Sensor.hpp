#ifndef CO2SENSOR_HPP
#define CO2SENSOR_HPP

#include <string>
#include <boost/asio.hpp>

class CO2Sensor{

public:

    // Constructor
    CO2Sensor(const std::string& port);

    // Destructor
    ~CO2Sensor();

    // Read CO2 concentration
    int readCO2();

    // ZERO POINT CALIBRATION
    // NOTE：ZERO POINT is 400PPM, 
    // PLS MAKE SURE THE SENSOR HAD BEEN WORKED 
    // UNDER 400PPM FOR OVER 20MINUTES
    void calibrateZero();

    // SPAN is 2000ppm，HIGH = 2000 / 256；LOW = 2000 % 256
    // Pls do ZERO calibration before span calibration
    // Suggest using 2000ppm as span, at least 1000ppm
    void calibrateSpan();

    // All Winsen sensor with ABC logic on before delivery.
    void onAutoCalibration();

    void offAutoCalibration();

    // Detection range is 2000 or 5000ppm
    void setDetectionRange();

    // Checksum to verify data receipt
    char getCheckSum();

private:

    boost::asio::io_service io;

    boost::asio::serial_port UARTPort;

};

#endif