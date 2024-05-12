#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include <vector>

#include "Data.hpp"

using namespace boost::asio;
using namespace boost::asio::ip;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(io_context &io_context, tcp::socket &&socket, DoTaskCallback doTaskCallback,
            unsigned int sessionId);

    void start();
    unsigned int getId() const;

private:
    void doRead();
    void doWrite(const std::string &response);

    tcp::socket socket_;
    std::vector<char> data_;
    DoTaskCallback doTaskCallback_;
    unsigned int sessionId_;
};

#endif // SESSION_HPP
