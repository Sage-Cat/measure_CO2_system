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
    std::cout << "Usage: " << progName
              << " [-s sensorPath] [-i measuringInterval] [-a ipAddress] [-p port]\n"
              << "Options:\n"
              << "  -s, --sensor       Sensor path (default: /dev/ttyAMA0)\n"
              << "  -i, --interval     Measuring interval in seconds (default: 10)\n"
              << "  -a, --address      IP address to bind to (default: 10.10.10.112)\n"
              << "  -p, --port         Port to listen on (default: 12345)\n";
}

int main(int argc, char **argv)
{
    SpdlogConfig::init<SpdlogConfig::LogLevel::Trace>();

    // Default values
    std::string sensorPath = "/dev/ttyAMA0";
    int measuringInterval  = 10;
    std::string ip_address = "10.10.10.112";
    unsigned short port    = 12345;

    // Argument parsing using getopt
    int opt;
    while ((opt = getopt(argc, argv, "s:i:a:p:")) != -1) {
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
        default:
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (measuringInterval <= 0) {
        std::cerr << "Measuring interval must be a positive integer.\n";
        return EXIT_FAILURE;
    }

    // Main logic
    SQLiteDatabase db(DATABASE_FILE_PATH); // create/open db near exe file

    CO2Sensor sensor(sensorPath);

    Application application(sensor, db, std::chrono::seconds(measuringInterval));

    io_context ioContext;
    ip::tcp::endpoint endpoint(ip::address::from_string(ip_address), port);
    Server server(ioContext, endpoint,
                  [&application](RequestData data, SendResponseCallback callback) {
                      SPDLOG_TRACE("DoTaskCallback");
                      application.doTask(data, callback);
                  });

    server.startAccept();

    ioContext.run();

    return 0;
}
