#include "Application.hpp"

#include "Database/SQLiteDatabase.hpp"
#include "Devices/CO2Sensor.hpp"
#include "Devices/LED.hpp"

#include "SpdlogConfig.hpp"
#include "Utils.hpp"

Application::Application(CO2Sensor &co2Sensor, LED &led, SQLiteDatabase &db,
                         std::chrono::seconds measuring_interval)
    : sensor_(co2Sensor), led_(led), db_(db), stopThreads_(false),
      measuringInterval_(measuring_interval)
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

    try {
        if (data.cmd == "get_indoor") {
            resData.measurements = db_.getIndoorCO2Samples();
        } else if (data.cmd == "get_indoor_after") {
            if (data.params.size() < 1) {
                throw std::invalid_argument("Missing parameter for 'get_indoor_after'");
            }
            resData.measurements = db_.getIndoorCO2SamplesAfterDatetime(data.params.at(0));
        } else if (data.cmd == "get_outdoor") {
            resData.measurements = db_.getOutdoorCO2Samples();
        } else if (data.cmd == "warning_on") {
            led_.on();
        } else if (data.cmd == "warning_off") {
            led_.off();
        } else if (data.cmd == "get_forecast") {
            resData.measurements = makeForecast(db_.getIndoorCO2Samples());
        } else {
            SPDLOG_WARN("Application::doTask | unknown cmd: {}", data.cmd);
        }
    } catch (const std::runtime_error &e) {
        SPDLOG_ERROR("Application::doTask | Error processing cmd: {} | RuntimeException: {}",
                     data.cmd, e.what());
        resData.error = std::string("Runtime error: ") + e.what();
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Application::doTask | Error processing cmd: {} | Exception: {}", data.cmd,
                     e.what());
        resData.error = std::string("Error: ") + e.what();
    }

    callback(resData);
}

void Application::setFetchOutdoorCO2Callback(FetchOutdoorCO2TaskCallback callback)
{
    fetchOutdoorCO2Callback_ = callback;
}

void Application::startTasks()
{
    SPDLOG_TRACE("Application::startThreads");

    sensorThread_  = std::thread([this]() { this->sensorTask(); });
    outdoorThread_ = std::thread([this]() { this->outdoorTask(); });
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

std::vector<CO2Sample> Application::makeForecast(std::vector<CO2Sample> measurements) {
    SPDLOG_TRACE("Application::makeForecast");

    BoosterHandle booster;
    XGBoosterCreate(nullptr, 0, &booster);
    XGBoosterLoadModel(booster, XGMODEL_FILE_PATH);

    std::vector<float> features;
    for (const auto& sample : measurements) {
        tm datetime = {};
        std::istringstream ss(sample.datetime);
        ss >> std::get_time(&datetime, "%Y-%m-%d %H:%M:%S");

        features.push_back(static_cast<float>(datetime.tm_hour));
        features.push_back(static_cast<float>(datetime.tm_mday));
        features.push_back(static_cast<float>(datetime.tm_mon + 1));
        features.push_back(static_cast<float>(datetime.tm_wday));
    }

    int num_features = 4;
    int num_samples = measurements.size();
    std::vector<float> X_flat(features.begin(), features.end());

    DMatrixHandle h_input;
    XGDMatrixCreateFromMat(X_flat.data(), num_samples, num_features, -1, &h_input);

    bst_ulong out_len;
    const float* y_pred;
    XGBoosterPredict(booster, h_input, 0, 0, 0, &out_len, &y_pred);

    std::vector<CO2Sample> forecasted_samples;
    for (int i = 0; i < 24; ++i) {
        CO2Sample sample;
        
        tm future_time = {};
        std::istringstream ss(measurements.back().datetime);
        ss >> std::get_time(&future_time, "%Y-%m-%d %H:%M:%S");
        future_time.tm_hour += i + 1;
        mktime(&future_time);
        
        std::ostringstream datetime_ss;
        datetime_ss << std::put_time(&future_time, "%Y-%m-%d %H:%M:%S");
        sample.datetime = datetime_ss.str();
        
        sample.CO2Level = std::to_string(y_pred[i]);
        forecasted_samples.push_back(sample);
    }

    XGDMatrixFree(h_input);
    XGBoosterFree(booster);

    return forecasted_samples;
}
