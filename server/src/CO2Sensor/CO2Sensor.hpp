#ifndef CO2SENSOR_HPP
#define CO2SENSOR_HPP

#include <array>
#include <boost/asio.hpp>
#include <string>

class CO2Sensor {
public:
    enum class Calibration : int { Span2000 = 2000, Span5000 = 5000 };
    static constexpr int DEFAULT_CO2 = 400;
    static constexpr int MIN_SPAN_PPM = 1000;

public:
    explicit CO2Sensor(const std::string &port);
    ~CO2Sensor();

    // Reads CO2 concentration in PPM using UART interface.
    int readCO2();

    // Calibrates the zero point assuming the sensor is in a 400PPM CO2 environment.
    void calibrateZero();

    // Calibrates the span. Precondition: perform zero point calibration first.
    void calibrateSpan(int span);

    // Toggles the automatic calibration logic (ABC) provided by the sensor.
    void enableAutoCalibration(bool enable);

    // Sets the detection range for the sensor to predefined spans.
    void setDetectionRange(Calibration range);

private:
    void sendCommand();
    char calculateChecksum() const;

private:
    boost::asio::io_service io_;
    boost::asio::serial_port uart_port_;
    std::array<unsigned char, 9> command_;
};

#endif // CO2SENSOR_HPP
