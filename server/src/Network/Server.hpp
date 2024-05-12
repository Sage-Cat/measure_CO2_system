#ifndef SERVER_HPP
#define SERVER_HPP

#include <memory>
#include <unordered_map>

#include <boost/asio.hpp>

#include "Session.hpp"

using namespace boost::asio;
using namespace boost::asio::ip;

class Server {
public:
    Server(io_context &io_context, const tcp::endpoint &endpoint, DoTaskCallback doTaskCallback);

    void startAccept();

private:
    void handleAccept(const boost::system::error_code &error, std::shared_ptr<tcp::socket> socket);

private:
    io_context &ioContext_;
    tcp::acceptor acceptor_;
    std::unordered_map<unsigned int, std::shared_ptr<Session>> sessions;

    DoTaskCallback doTaskCallback_;
};

#endif // SERVER_HPP
