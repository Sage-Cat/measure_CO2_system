#include "Session.hpp"
#include "SpdlogConfig.hpp"

Session::Session(io_context &io_context, tcp::socket &&socket,
                 ProcessDataCallback processDataCallback, unsigned int sessionId)
    : socket_(std::move(socket)), processDataCallback_(processDataCallback), sessionId_(sessionId),
      data_(1024)
{
    SPDLOG_INFO("Session {} created", sessionId_);
}

void Session::start() { doRead(); }

unsigned int Session::getId() const { return sessionId_; }

void Session::doRead()
{
    SPDLOG_TRACE("Session::doRead");
    auto self(shared_from_this());
    socket_.async_read_some(buffer(data_), [this, self](boost::system::error_code ec,
                                                        std::size_t length) {
        if (!ec) {
            processDataCallback_(self, std::vector<char>(data_.begin(), data_.begin() + length));
            doWrite(length);
        } else {
            SPDLOG_ERROR("Session {}: Read error: {}", sessionId_, ec.message());
        }
    });
}

void Session::doWrite(std::size_t length)
{
    SPDLOG_TRACE("Session::doWrite");
    auto self(shared_from_this());
    async_write(socket_, buffer(data_, length),
                [this, self](boost::system::error_code ec, std::size_t) {
                    if (!ec) {
                        SPDLOG_INFO("Session {}: Data written successfully", sessionId_);
                        doRead();
                    } else {
                        SPDLOG_ERROR("Session {}: Write error: {}", sessionId_, ec.message());
                    }
                });
}
