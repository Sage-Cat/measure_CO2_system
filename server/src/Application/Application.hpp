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
    Application(CO2Sensor &co2Sensor, SQLiteDatabase &db, std::chrono::seconds measuring_interval);
    ~Application();

    void doTask(RequestData data, SendResponseCallback callback);
    void setFetchOutdoorCO2Callback(FetchOutdoorCO2TaskCallback callback);
    void startTasks();

private:
    void sensorTask();
    void outdoorCO2Task();
    std::string getCurrentDateTime();

private:
    CO2Sensor &sensor_;
    SQLiteDatabase &db_;

    std::thread sensorThread_;
    std::thread outdoorThread_;
    std::atomic<bool> stopThreads_;
    std::chrono::seconds measuringInterval_;

    FetchOutdoorCO2TaskCallback fetchOutdoorCO2Callback_;
};

#endif // APPLICATION_HPP
