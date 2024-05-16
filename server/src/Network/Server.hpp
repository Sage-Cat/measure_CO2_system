#ifndef SERVER_HPP
#define SERVER_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <unordered_map>

#include "Session.hpp"

class Server {
public:
    Server(boost::asio::io_context &io_context, const boost::asio::ip::tcp::endpoint &endpoint,
           const std::string &openWeatherApiKey, DoTaskCallback doTaskCallback);

    std::string fetchOutdoorCO2Level(const std::string &location);

private:
    void startAccept();
    void handleAccept(const boost::system::error_code &error,
                      std::shared_ptr<boost::asio::ip::tcp::socket> socket);

private:
    boost::asio::io_context &ioContext_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::unordered_map<unsigned int, std::shared_ptr<Session>> sessions;
    DoTaskCallback doTaskCallback_;

    std::string openWeatherApiKey_;
};

#endif // SERVER_HPP
