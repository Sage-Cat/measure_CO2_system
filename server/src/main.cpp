#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>
#include <vector>

#include "Application/Application.hpp"
#include "CO2Sensor/CO2Sensor.hpp"
#include "Database/SQLiteDatabase.hpp"
#include "Network/Server.hpp"
#include "SpdlogConfig.hpp"

using namespace boost::asio;

void printUsage(const char *progName)
{
    std::cout << "Usage: " << progName << " [-s sensorPath] [-i measuringInterval]\n"
              << "Options:\n"
              << "  -s, --sensor       Sensor path (default: /dev/ttyAMA0)\n"
              << "  -i, --interval     Measuring interval in seconds (default: 10)\n";
}

int main(int argc, char **argv)
{
    SpdlogConfig::init<SpdlogConfig::LogLevel::Trace>();

    const std::string newHostname = "co2measure.local";

    // Default values
    std::string sensorPath = "/dev/ttyAMA0";
    int measuringInterval  = 10;

    // Argument parsing using getopt
    int opt;
    while ((opt = getopt(argc, argv, "s:i:")) != -1) {
        switch (opt) {
        case 's':
            sensorPath = optarg;
            break;
        case 'i':
            measuringInterval = std::stoi(optarg);
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

    if (std::system(("sudo hostnamectl set-hostname " + newHostname).c_str()) != 0) {
         std::cerr << "Error setting hostname. Please check for sufficient permissions.\n";
        return EXIT_FAILURE;
    }

    // Main logic
    SQLiteDatabase db(DATABASE_FILE_PATH); // create/open db near exe file

    CO2Sensor sensor(sensorPath);

    Application application(sensor, db, std::chrono::seconds(measuringInterval));

    io_context ioContext;
    ip::tcp::endpoint endpoint(ip::tcp::v4(), 12345);
    Server server(ioContext, endpoint,
                  [&application](RequestData data, SendResponseCallback callback) {
                      SPDLOG_TRACE("DoTaskCallback");
                      application.doTask(data, callback);
                  });

    server.startAccept();

    ioContext.run();

    return 0;
}
