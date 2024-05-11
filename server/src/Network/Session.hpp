#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include <vector>

using namespace boost::asio;
using namespace boost::asio::ip;

class Session : public std::enable_shared_from_this<Session> {
public:
    using ProcessDataCallback = std::function<void(std::shared_ptr<Session>, std::vector<char>)>;

    Session(io_context &io_context, tcp::socket &&socket, ProcessDataCallback processDataCallback,
            unsigned int sessionId);

    void start();
    unsigned int getId() const;

private:
    void doRead();
    void doWrite(std::size_t length);

    tcp::socket socket_;
    std::vector<char> data_;
    ProcessDataCallback processDataCallback_;
    unsigned int sessionId_;
};

#endif // SESSION_HPP
