#ifndef SESSION_HPP
#define SESSION_HPP

#include "Data/Data.hpp"
#include <boost/asio.hpp>


using namespace boost::asio;

class Session : public std::enable_shared_from_this<Session> {
public:
  
  using SessProcDataCallback = std::function<void(std::string, Data)>;

  Session(io_service& io_service, SessProcDataCallback sessProcDataCallback);
  ip::tcp::socket &socket();
  void start();

private:
  void handleRead(std::size_t length);

  ip::tcp::socket socket_;
  char receivedData_[512];

  SessProcDataCallback sessProcDataCallback_;
};

#endif