#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/asio.hpp>
#include "Data/Data.hpp"

using namespace boost::asio;

class Session : public std::enable_shared_from_this<Session> {
public:

    using AppCallback = std::function<void(std::function<void(Data)>)>;

    Session(io_context& ioContext, AppCallback appCallback);
    ip::tcp::socket& socket();
    void start();

private:
    void handleRead(std::size_t length);

    ip::tcp::socket socket_;
    char data_[512];

    AppCallback appCallback;
};

#endif
