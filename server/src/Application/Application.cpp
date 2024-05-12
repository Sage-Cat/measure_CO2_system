#include "Application.hpp"

Application::Application(CO2Sensor &co2Sensor, SQLiteDatabase &sqliteDatabase)
    : sensor(co2Sensor), database(sqliteDatabase)
{
}

Application::~Application() {}

void Application::doTask(Callback callback)
{
    Data resData;

    // TODO: Perform data analysis using machine learning algorithms

    callback(resData);
}