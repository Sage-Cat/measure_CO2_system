#include "Application.hpp"

#include "CO2Sensor/CO2Sensor.hpp"
#include "Database/SQLiteDatabase.hpp"

#include "SpdlogConfig.hpp"

Application::Application(CO2Sensor &co2Sensor, SQLiteDatabase &db)
    : sensor_(co2Sensor), db_(db), sensorThread([this]() { this->sensorTask(); })
{
    SPDLOG_TRACE("Application::Application");
}

void Application::doTask(RequestData data, SendResponseCallback callback)
{
    SPDLOG_TRACE("Application::doTask | cmd - {}", data.cmd);
    ResponseData resData{};

    if (data.cmd == "get_all") {
        // geting all records using the most early date
        resData.measurements = db_.getMeasurementsAfterDate("");
    } else if (data.cmd == "get_after") {
        resData.measurements = db_.getMeasurementsAfterDate(data.param1);
    } else {
        SPDLOG_WARN("Application::doTask | unknown cmd");
    }

    callback(resData);
}

void Application::sensorTask()
{
    try {
        while (true) {
            CO2Sample co2sample{getCurrentDateTime(), std::to_string(sensor_.readCO2())};
            db_.addMeasurement(co2sample);

            std::this_thread::sleep_for(std::chrono::seconds(TIME_BETWEEN_MEASURING));
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception in sensorTask: {}", e.what());
    }
}

std::string Application::getCurrentDateTime()
{
    auto now       = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm     = *std::localtime(&in_time_t);

    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}