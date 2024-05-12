#include "Server.hpp"

#include <atomic>
#include <memory>

#include "SpdlogConfig.hpp"

using namespace boost::asio;
using namespace boost::asio::ip;

static std::atomic<unsigned int> nextSessionId{1};

Server::Server(io_context &io_context, const tcp::endpoint &endpoint,
               DoTaskCallback procDataCallback)
    : ioContext_(io_context), acceptor_(io_context, endpoint), doTaskCallback_(procDataCallback)
{
    SPDLOG_INFO("Server starting on address {}, port {}", endpoint.address().to_string(),
                endpoint.port());
    startAccept();
}

void Server::startAccept()
{
    SPDLOG_TRACE("Server::startAccept");
    auto socket = std::make_shared<tcp::socket>(ioContext_);
    acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code &error) {
        handleAccept(error, socket);
    });
}

void Server::handleAccept(const boost::system::error_code &error,
                          std::shared_ptr<tcp::socket> socket)
{
    SPDLOG_TRACE("Server::handleAccept");
    if (!error) {
        SPDLOG_INFO("Connection accepted");
        unsigned int sessionId = nextSessionId++;
        auto session =
            std::make_shared<Session>(ioContext_, std::move(*socket), doTaskCallback_, sessionId);
        session->start();
        sessions[sessionId] = session;
    } else {
        SPDLOG_ERROR("Error on accept: {}", error.message());
    }
    startAccept();
}
