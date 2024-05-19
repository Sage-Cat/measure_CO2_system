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
              << " [-s sensorPath] [-i measuringInterval] [-a ipAddress] [-p port] [-k "
                 "apiKeyFilePath] [-l ledPin]\n"
              << "Options:\n"
              << "  -a, --address      IP address to bind to (default: 10.10.10.112)\n"
              << "  -p, --port         Port to listen on (default: 12345)\n"
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

    const std::string newHostname = "co2measure.local";

    // Default values
    std::string sensorPath     = "/dev/ttyAMA0";
    int measuringInterval      = 10;
    std::string ip_address     = "10.10.10.112";
    unsigned short port        = 12345;
    std::string apiKeyFilePath = "./open_weather_api_key.txt";
    int ledPin                 = 18;

    // Argument parsing using getopt
    int opt;
    while ((opt = getopt(argc, argv, "s:i:a:p:k:l:")) != -1) {
        switch (opt) {
        case 's':
            sensorPath = optarg;
            break;
        case 'i':
            measuringInterval = std::stoi(optarg);
            break;
        case 'a':
            ip_address = optarg;
            break;
        case 'p':
            port = static_cast<unsigned short>(std::stoi(optarg));
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
        std::cerr << "Measuring interval must be a positive integer.\n";
        return EXIT_FAILURE;
    }

<<<<<<< HEAD
    // Read the OpenWeather API key
    std::string openWeatherApiKey{};
    try {
        openWeatherApiKey = readApiKeyFromFile(apiKeyFilePath);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
=======
    if (std::system(("sudo hostnamectl set-hostname " + newHostname).c_str()) != 0) {
         std::cerr << "Error setting hostname. Please check for sufficient permissions.\n";
>>>>>>> b89ddd5 (Implemented singleton pattern for network connection)
        return EXIT_FAILURE;
    }

    // Main logic
    SQLiteDatabase db(DATABASE_FILE_PATH);
    CO2Sensor sensor(sensorPath);
    LED led(ledPin);

    Application application(sensor, led, db, std::chrono::seconds(measuringInterval));

    io_context ioContext;
    ip::tcp::endpoint endpoint(ip::address::from_string(ip_address), port);
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
    return 0;
}
