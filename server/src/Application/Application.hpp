#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <atomic>
#include <chrono>
#include <functional>
#include <iomanip>
#include <sstream>
#include <thread>

#include "Data.hpp"

class CO2Sensor;
class SQLiteDatabase;

class Application {
public:
    Application(CO2Sensor &co2Sensor, SQLiteDatabase &db,
                std::chrono::seconds measuring_interval);
    ~Application();

    void doTask(RequestData data, SendResponseCallback callback);

private:
    void sensorTask();
    std::string getCurrentDateTime();

private:
    CO2Sensor &sensor_;
    SQLiteDatabase &db_;

    std::thread sensorThread_;
    std::atomic<bool> stopThread_;
    std::chrono::seconds measuringInterval_;
};

#endif // APPLICATION_HPP
