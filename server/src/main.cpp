#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "Application/Application.hpp"
#include "CO2Sensor/CO2Sensor.hpp"
#include "Database/SQLiteDatabase.hpp"
#include "Network/Server.hpp"

#include "SpdlogConfig.hpp"

using namespace boost::asio;

int main()
{
    SpdlogConfig::init<SpdlogConfig::LogLevel::Trace>();

    io_context ioContext;
    ip::tcp::endpoint endpoint(ip::tcp::v4(), 12345);

    Server server(
        ioContext, endpoint, [](std::shared_ptr<Session> session, std::vector<char> responseData) {
            SPDLOG_INFO("Received data: {}", std::string(responseData.begin(), responseData.end()));
        });

    // SQLiteDatabase db(nullptr);
    // CO2Sensor sensor("/dev/ttyS0");

    // Application app(sensor, db);

    server.startAccept();

    ioContext.run();

    return 0;
}