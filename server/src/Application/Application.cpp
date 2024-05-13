#include "Application.hpp"

#include "CO2Sensor/CO2Sensor.hpp"
#include "Database/SQLiteDatabase.hpp"

#include "SpdlogConfig.hpp"
#include "Utils.hpp"

Application::Application(CO2Sensor &co2Sensor, SQLiteDatabase &db,
                         std::chrono::seconds measuring_interval)
    : sensor_(co2Sensor), db_(db), stopThread_(false), measuringInterval_(measuring_interval),
      sensorThread_([this]() { this->sensorTask(); })
{
    SPDLOG_TRACE("Application::Application");
}

Application::~Application()
{
    stopThread_ = true;
    if (sensorThread_.joinable()) {
        sensorThread_.join();
    }
}

void Application::doTask(RequestData data, SendResponseCallback callback)
{
    SPDLOG_TRACE("Application::doTask | cmd - {}", data.cmd);
    ResponseData resData{};

    if (data.cmd == "get_all") {
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
<<<<<<< HEAD
        while (true) {
            const auto current_CO2 = sensor_.readCO2(); 
            SPDLOG_INFO("Current CO2 Level - ", current_CO2);
            CO2Sample co2sample{getCurrentDateTime(), std::to_string(current_CO2)};
=======
        while (!stopThread_) {
            CO2Sample co2sample{Utils::getCurrentDateTime(), std::to_string(sensor_.readCO2())};
>>>>>>> 8cce645 (Refactor Application sensor)
            db_.addMeasurement(co2sample);

            std::this_thread::sleep_for(measuringInterval_);
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception in sensorTask: {}", e.what());
    }
}
