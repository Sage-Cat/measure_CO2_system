#ifndef SERVER_HPP
#define SERVER_HPP

#include <memory>
#include <vector>

#include <boost/asio.hpp>

#include "Session.hpp"

using namespace boost::asio;
using namespace boost::asio::ip;

class Server {
public:
    Server(io_context &io_context, const tcp::endpoint &endpoint,
           Session::ProcessDataCallback procDataCallback);

    void startAccept();

private:
    void handleAccept(const boost::system::error_code &error, std::shared_ptr<Session> session);

private:
    io_context &ioContext_;
    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<Session>> sessions;

    Session::ProcessDataCallback procDataCallback_;
};

#endif