#include "Session.hpp"

Session::Session(io_service& io_service, SessProcDataCallback sessProcDataCallback)
    : socket_(io_service), sessProcDataCallback_(sessProcDataCallback) {}

ip::tcp::socket &Session::socket() { return socket_; }

void Session::start() {
  async_read(socket_, buffer(receivedData_, 512),
             [self = shared_from_this()](boost::system::error_code ec,
                                         std::size_t length) {
               if (!ec) {
                 self->handleRead(length);
               }
             });
}

void Session::handleRead(std::size_t length) {
  async_write(
      socket_, buffer(receivedData_, length),
      [self = shared_from_this()](boost::system::error_code ec, std::size_t) {
        if (!ec) {
          self->start();
        }
      });
}