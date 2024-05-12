#include "Application.hpp"

Application::Application(CO2Sensor &co2Sensor, SQLiteDatabase &sqliteDatabase)
    : sensor(co2Sensor), database(sqliteDatabase)
{
}

void Application::doTask(RequestData data, SendResponseCallback callback)
{
    ResponseData resData{};

    callback(resData);
}