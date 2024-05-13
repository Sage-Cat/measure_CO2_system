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

    SQLiteDatabase db(DATABASE_FILE_PATH); // create/open db near exe file
    
    CO2Sensor sensor("/dev/pts2"); // use /dev/ttyAMA0 on RPI4B

    Application app(sensor, db);

    io_context ioContext;
    ip::tcp::endpoint endpoint(ip::tcp::v4(), 12345);
    Server server(ioContext, endpoint, [&app](RequestData data, SendResponseCallback callback) {
        SPDLOG_TRACE("DoTaskCallback");
        app.doTask(data, callback);
    });

    server.startAccept();

    ioContext.run();

    return 0;
}