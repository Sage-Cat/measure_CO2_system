#include "Session.hpp"

Session::Session(boost::asio::io_context& ioContext, AppCallback appCallback) 
    : socket_(ioContext), appCallback(appCallback) {}

ip::tcp::socket& Session::socket() {
    return socket_;
}

void Session::start() {
    async_read(socket_, buffer(data_, 512),
        [self = shared_from_this()](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                self->handleRead(length);
            }
        });
}

void Session::handleRead(std::size_t length) {
    async_write(socket_, buffer(data_, length),
        [self = shared_from_this()](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                self->start();
            }
        });
}