#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <functional>
#include <thread>
#include <iomanip>
#include <sstream>
#include <chrono>

#include "Data.hpp"

#define TIME_BETWEEN_MEASURING 120

class CO2Sensor;
class SQLiteDatabase;

class Application {
public:
    Application(CO2Sensor &co2Sensor, SQLiteDatabase &db);
    ~Application() = default;

    void doTask(RequestData data, SendResponseCallback callback);

private:

    
    CO2Sensor &sensor_;
    SQLiteDatabase &db_;

    std::jthread sensorThread;

    void sensorTask();

    std::string getCurrentDateTime();
};

#endif