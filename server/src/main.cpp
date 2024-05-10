#include "Application/Application.hpp"
#include "CO2Sensor/CO2Sensor.hpp"
#include "Database/SQLiteDatabase.hpp"
#include "Server.hpp"
#include <iostream>

using namespace boost::asio;

int main() {

  io_context ioContext;
  ip::tcp::endpoint endpoint(ip::tcp::v4(), 12345);

  Server server(ioContext, endpoint,
                [](std::string message, Data responseData) {
                  std::cout << "Received data from Application\n";
                });

  SQLiteDatabase db(nullptr);
  CO2Sensor sensor("/dev/ttyS0");

  Application app(sensor, db);

  server.startAccept();

  ioContext.run();

  return 0;
}