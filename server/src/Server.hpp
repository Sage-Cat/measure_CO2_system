#ifndef SERVER_HPP
#define SERVER_HPP

#include "Session.hpp"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <vector>

using namespace boost::asio;
using namespace boost::placeholders;

class Server {
public:
  Server(io_service& io_service, const ip::tcp::endpoint &endpoint,
         ProcessDataCallback procDatacallback);

  void startAccept();

private:
  void handleAccept(std::shared_ptr<Session> session,
                    const boost::system::error_code &error);

  io_service& ioService_;
  ip::tcp::acceptor acceptor_;
  std::vector<std::shared_ptr<Session>> sessions;

  ProcessDataCallback procDatacallback_;
};

#endif