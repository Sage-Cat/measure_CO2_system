#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <functional>

#include "Data.hpp"

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
};

#endif