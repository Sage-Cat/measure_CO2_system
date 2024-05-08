#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include "Session.hpp"

using namespace boost::asio;


class Server {
public:

    Server(io_context& ioContext, const ip::tcp::endpoint& endpoint, Callback appCallback);

    void startAccept();

private:

    void handleAccept(std::shared_ptr<Session> session, const boost::system::error_code& error);

    ip::tcp::acceptor acceptor;
    std::vector<std::shared_ptr<Session>> sessions;

    Callback callback;
};

#endif