#include "Application.hpp"

#include "Database/SQLiteDatabase.hpp"

#include "SpdlogConfig.hpp"

Application::Application(CO2Sensor &co2Sensor, SQLiteDatabase &db)
    : sensor_(co2Sensor), db_(db)
{
    SPDLOG_TRACE("Application::Application");
}

void Application::doTask(RequestData data, SendResponseCallback callback)
{
    SPDLOG_TRACE("Application::doTask | cmd - {}", data.cmd);
    ResponseData resData{};

    if (data.cmd == "get_all") 
    { 
        // geting all records using the most early date
        resData.measurements = db_.getMeasurementsAfterDate("");
    } else if (data.cmd == "get_after") {
        resData.measurements = db_.getMeasurementsAfterDate(data.param1);
    } else {
        SPDLOG_WARN("Application::doTask | unknown cmd");
    }

    callback(resData);
}