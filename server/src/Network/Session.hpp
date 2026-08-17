#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "Data.hpp"

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(boost::asio::ip::tcp::socket socket, DoTaskCallback doTaskCallback,
            unsigned int sessionId);

    void start();
    unsigned int getId() const;

private:
    void doRead();
    void doWrite(boost::beast::http::status status, const std::string &response);
    void doClose();

private:
    boost::beast::tcp_stream socket_;
    boost::beast::flat_buffer buffer_;
    DoTaskCallback doTaskCallback_;
    unsigned int sessionId_;
};

#endif // SESSION_HPP
