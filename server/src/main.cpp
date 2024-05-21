#include <boost/asio.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>
#include <vector>

#include "Application.hpp"
#include "Database/SQLiteDatabase.hpp"
#include "Devices/CO2Sensor.hpp"
#include "Devices/LED.hpp"
#include "Network/Server.hpp"

#include "SpdlogConfig.hpp"

using namespace boost::asio;

void printUsage(const char *progName)
{
    std::cout << "Usage: " << progName
              << " [-s sensorPath] [-i measuringInterval] [-k apiKeyFilePath] [-l ledPin]\n"
              << "Options:\n"
              << "  -s, --sensor       Sensor path (default: /dev/ttyAMA0)\n"
              << "  -i, --interval     Measuring interval in seconds (default: 10)\n"
              << "  -k, --apikey       Path to the OpenWeather API key file (default: "
                 "./open_weather_api.txt)\n"
              << "  -l, --led          GPIO pin for the LED (default: 18)\n";
}

std::string readApiKeyFromFile(const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open API key file: " + filePath);
    }

    std::string apiKey;
    std::getline(file, apiKey);

    if (apiKey.empty()) {
        throw std::runtime_error("API key file is empty: " + filePath);
    }

    return apiKey;
}

int main(int argc, char **argv)
{
    SpdlogConfig::init<SpdlogConfig::LogLevel::Trace>();

    const std::string hostname = "co2measure.local";

    // Default values
    std::string sensorPath     = "/dev/ttyAMA0";
    int measuringInterval      = 10;
    unsigned short port        = 12345;
    std::string apiKeyFilePath = "./open_weather_api_key.txt";
    int ledPin                 = 18;

    // Argument parsing using getopt
    int opt;
    while ((opt = getopt(argc, argv, "s:i:k:l:")) != -1) {
        switch (opt) {
        case 's':
            sensorPath = optarg;
            break;
        case 'i':
            measuringInterval = std::stoi(optarg);
            break;
        case 'k':
            apiKeyFilePath = optarg;
            break;
        case 'l':
            ledPin = std::stoi(optarg);
            break;
        default:
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (measuringInterval <= 0) {
        SPDLOG_ERROR("Measuring interval must be a positive integer.");
        return EXIT_FAILURE;
    }

    // ---Network Configuration Update---

    // Sets a new hostname using hostnamectl for using mDNS
    int status = std::system(("sudo hostnamectl set-hostname " + hostname).c_str());
    if (status != 0) {
        SPDLOG_ERROR("Error setting hostname. Status: {}, Error: {}", status, strerror(errno));
        return EXIT_FAILURE;
    }

    // Restarts the avahi-daemon service to apply changes and update network information
    status = std::system("sudo systemctl restart avahi-daemon");
    if (status != 0) {
        SPDLOG_ERROR("Error restarting avahi-daemon. Status: {}, Error: {}", status,
                     strerror(errno));
        return EXIT_FAILURE;
    }

    // Read the OpenWeather API key
    std::string openWeatherApiKey{};
    try {
        openWeatherApiKey = readApiKeyFromFile(apiKeyFilePath);
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Error openWeather api key: Error: {}", e.what());
        return EXIT_FAILURE;
    }

    // Main logic
    try {
        SQLiteDatabase db(DATABASE_FILE_PATH);
        CO2Sensor sensor(sensorPath);
        LED led(ledPin);

        Application application(sensor, led, db, std::chrono::seconds(measuringInterval));

        io_context ioContext;
        ip::tcp::endpoint endpoint(ip::tcp::v4(), port);
        Server server(ioContext, endpoint, openWeatherApiKey,
                      [&application](RequestData data, SendResponseCallback callback) {
                          SPDLOG_TRACE("DoTaskCallback");
                          application.doTask(data, callback);
                      });

        application.setFetchOutdoorCO2Callback([&server](const std::string &location) {
            SPDLOG_TRACE("FetchOutdoorCO2Callback");
            return server.fetchOutdoorCO2Level(location);
        });
        application.startTasks();

        ioContext.run();
    } catch (const std::runtime_error &e) {
        SPDLOG_CRITICAL("Runtime error: {}", e.what());
        return EXIT_FAILURE;
    } catch (const std::exception &e) {
        SPDLOG_CRITICAL("Exception: {}", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        SPDLOG_CRITICAL("Unknown error occurred.");
        return EXIT_FAILURE;
    }

    return 0;
}
