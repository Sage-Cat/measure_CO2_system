#include "Server.hpp"

#include <atomic>
#include <memory>

#include "Session.hpp"
#include "SpdlogConfig.hpp"

using namespace boost::asio;
using namespace boost::asio::ip;

static std::atomic<unsigned int> nextSessionId{1};

Server::Server(io_context &io_context, const tcp::endpoint &endpoint,
               Session::ProcessDataCallback procDataCallback)
    : ioContext_(io_context), acceptor_(io_context, endpoint), procDataCallback_(procDataCallback)
{
    SPDLOG_INFO("Server initialized and starting accept");
    startAccept();
}

void Server::startAccept()
{
    SPDLOG_TRACE("Server::startAccept");
    auto socket = std::make_shared<tcp::socket>(ioContext_);
    acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code &error) {
        if (!error) {
            SPDLOG_INFO("Connection accepted");
            unsigned int sessionId = nextSessionId++;
            auto session           = std::make_shared<Session>(ioContext_, std::move(*socket),
                                                     procDataCallback_, sessionId);
            session->start();
            sessions.push_back(session);
        } else {
            SPDLOG_ERROR("Error on accept: {}", error.message());
        }
        startAccept();
    });
}

void Server::handleAccept(const boost::system::error_code &error, std::shared_ptr<Session> session)
{
    SPDLOG_TRACE("Server::handleAccept");
    if (!error) {
        session->start();
        sessions.push_back(session);
    } else {
        SPDLOG_ERROR("Error on accept: {}", error.message());
    }
}
