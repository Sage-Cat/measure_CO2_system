#include "Application.hpp"

#include "Database/SQLiteDatabase.hpp"
#include "Devices/CO2Sensor.hpp"
#include "Devices/LED.hpp"

#include "SpdlogConfig.hpp"
#include "Utils.hpp"

namespace {
void checkXgboost(int result, const char *operation)
{
    if (result != 0) {
        throw std::runtime_error(std::string(operation) + ": " + XGBGetLastError());
    }
}
} // namespace

Application::Application(CO2Sensor &co2Sensor, LED &led, SQLiteDatabase &db,
                         std::chrono::seconds measuring_interval)
    : sensor_(co2Sensor), led_(led), db_(db), booster_(nullptr), stopThreads_(false),
      measuringInterval_(measuring_interval)
{
    SPDLOG_TRACE("Application::Application");
    checkXgboost(XGBoosterCreate(nullptr, 0, &booster_), "creating XGBoost booster");
    try {
        checkXgboost(XGBoosterLoadModel(booster_, XGMODEL_FILE_PATH), "loading XGBoost model");
    } catch (...) {
        XGBoosterFree(booster_);
        booster_ = nullptr;
        throw;
    }
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

    if (booster_) {
        XGBoosterFree(booster_);
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
            resData.error = "Unknown command";
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

void Application::setFetchOutdoorCOCallback(FetchOutdoorCOTaskCallback callback)
{
    fetchOutdoorCOCallback_ = callback;
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
            if (fetchOutdoorCOCallback_) {
                const auto current_CO = fetchOutdoorCOCallback_("");
                SPDLOG_INFO("Current outdoor CO level (µg/m³): {}", current_CO);
                CO2Sample co2sample{Utils::getCurrentDateTime(), current_CO};
                db_.addOutdoorCO2Sample(co2sample);
            } else {
                SPDLOG_ERROR("fetchOutdoorCOCallback_ is not set");
                break;
            }
            std::this_thread::sleep_for(measuringInterval_);
        }
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Exception in outdoorTask: {}", e.what());
    }
}

std::vector<CO2Sample> Application::makeForecast(std::vector<CO2Sample> measurements)
{
    SPDLOG_TRACE("Application::makeForecast");

    if (measurements.empty()) {
        throw std::runtime_error("Forecast requires at least one indoor sample");
    }

    std::lock_guard<std::mutex> lock(forecastMutex_);

    std::vector<float> features;
    std::vector<std::string> forecastTimes;
    std::tm futureTime{};
    std::istringstream input(measurements.back().datetime);
    input >> std::get_time(&futureTime, "%Y-%m-%d %H:%M:%S");
    if (input.fail()) {
        throw std::runtime_error("Latest sample has an invalid datetime");
    }

    for (int i = 0; i < 24; ++i) {
        futureTime.tm_hour += 1;
        if (std::mktime(&futureTime) == -1) {
            throw std::runtime_error("Could not calculate forecast datetime");
        }
        features.push_back(static_cast<float>(futureTime.tm_hour));
        features.push_back(static_cast<float>(futureTime.tm_mday));
        features.push_back(static_cast<float>(futureTime.tm_mon + 1));
        features.push_back(static_cast<float>(futureTime.tm_wday));
        std::ostringstream output;
        output << std::put_time(&futureTime, "%Y-%m-%d %H:%M:%S");
        forecastTimes.push_back(output.str());
    }

    DMatrixHandle inputMatrix = nullptr;
    checkXgboost(XGDMatrixCreateFromMat(features.data(), 24, 4, -1, &inputMatrix),
                 "creating forecast matrix");

    bst_ulong out_len;
    const float *y_pred;
    const int predictResult = XGBoosterPredict(booster_, inputMatrix, 0, 0, 0, &out_len, &y_pred);
    XGDMatrixFree(inputMatrix);
    checkXgboost(predictResult, "running forecast");
    if (out_len < 24) {
        throw std::runtime_error("Forecast model returned fewer than 24 predictions");
    }

    std::vector<CO2Sample> forecasted_samples;
    for (int i = 0; i < 24; ++i) {
        forecasted_samples.push_back(CO2Sample{forecastTimes.at(i), std::to_string(y_pred[i])});
    }

    return forecasted_samples;
}
