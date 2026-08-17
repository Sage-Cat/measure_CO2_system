#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <atomic>
#include <chrono>
#include <functional>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <thread>
#include <xgboost/c_api.h>

#include "Data.hpp"

class CO2Sensor;
class LED;
class SQLiteDatabase;

class Application {
public:
    Application(CO2Sensor &co2Sensor, LED &led, SQLiteDatabase &db,
                std::chrono::seconds measuring_interval);
    ~Application();

    void doTask(RequestData data, SendResponseCallback callback);
    void setFetchOutdoorCOCallback(FetchOutdoorCOTaskCallback callback);
    void startTasks();

private:
    void sensorTask();
    void outdoorTask();
    std::string getCurrentDateTime();
    std::vector<CO2Sample> makeForecast(std::vector<CO2Sample> measurements);

private:
    CO2Sensor &sensor_;
    LED &led_;
    SQLiteDatabase &db_;

    BoosterHandle booster_;
    std::mutex forecastMutex_;

    std::thread sensorThread_;
    std::thread outdoorThread_;
    std::atomic<bool> stopThreads_;
    std::chrono::seconds measuringInterval_;

    FetchOutdoorCOTaskCallback fetchOutdoorCOCallback_;
};

#endif // APPLICATION_HPP
