#include "Application.hpp"

#include "Database/SQLiteDatabase.hpp"
#include "Devices/CO2Sensor.hpp"
#include "Devices/LED.hpp"

#include "SpdlogConfig.hpp"
#include "Utils.hpp"

Application::Application(CO2Sensor &co2Sensor, LED &led, SQLiteDatabase &db,
                         std::chrono::seconds measuring_interval)
    : sensor_(co2Sensor), led_(led), db_(db), stopThreads_(false),
      measuringInterval_(measuring_interval), sensorThread_([this]() { this->sensorTask(); }),
      outdoorThread_([this]() { this->outdoorTask(); })
{
    SPDLOG_TRACE("Application::Application");
}

Application::~Application()
{
    SPDLOG_TRACE("Application::~Application");
    stopThreads_ = true;
    if (sensorThread_.joinable()) {
        sensorThread_.join();
    }
    if (outdoorThread_.joinable()) {
        outdoorThread_.join();
    }
}

void Application::doTask(RequestData data, SendResponseCallback callback)
{
    SPDLOG_TRACE("Application::doTask | cmd - {}", data.cmd);
    ResponseData resData{};

    if (data.cmd == "get_indoor") {
        resData.measurements = db_.getIndoorCO2Samples();
    } else if (data.cmd == "get_indoor_after") {
        resData.measurements = db_.getIndoorCO2SamplesAfterDatetime(data.param1);
    } else if (data.cmd == "get_outdoor") {
        resData.measurements = db_.getOutdoorCO2Samples();
    } else if (data.cmd == "warning_on") {
        led_.on();
    } else if (data.cmd == "warning_off") {
        led_.off();
    } else {
        SPDLOG_WARN("Application::doTask | unknown cmd");
    }

    callback(resData);
}

void Application::setFetchOutdoorCO2Callback(FetchOutdoorCO2TaskCallback callback)
{
    fetchOutdoorCO2Callback_ = callback;
}

void Application::sensorTask()
{
    SPDLOG_TRACE("Application::sensorTask");
    try {
        while (!stopThreads_) {
            const auto current_CO2 = sensor_.readCO2();
            SPDLOG_INFO("Current Indoor CO2 Level: {}", current_CO2);
            CO2Sample co2sample{Utils::getCurrentDateTime(), std::to_string(current_CO2)};
            db_.addIndoorCO2Sample(co2sample);

            std::this_thread::sleep_for(measuringInterval_);
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception in sensorTask: {}", e.what());
    }
}

void Application::outdoorTask()
{
    SPDLOG_TRACE("Application::outdoorTask");
    try {
        while (!stopThreads_) {
            if (fetchOutdoorCO2Callback_) {
                const auto current_CO2 = fetchOutdoorCO2Callback_("");
                SPDLOG_INFO("Current Outdoor CO2 Level: {}", current_CO2);
                CO2Sample co2sample{Utils::getCurrentDateTime(), current_CO2};
                db_.addOutdoorCO2Sample(co2sample);
            } else {
                SPDLOG_ERROR("fetchOutdoorCO2Callback_ is not set");
                break;
            }
            std::this_thread::sleep_for(measuringInterval_);
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception in outdoorTask: {}", e.what());
    }
}
