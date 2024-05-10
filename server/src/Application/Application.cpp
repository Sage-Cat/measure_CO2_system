#include "Application.hpp"

Application::Application(CO2Sensor &co2Sensor, SQLiteDatabase &sqliteDatabase)
    : sensor(co2Sensor), database(sqliteDatabase) {}

Application::~Application() {}

void Application::doTask(Callback callback) {

  Data resData;

  // some work with data

  callback(resData);
}