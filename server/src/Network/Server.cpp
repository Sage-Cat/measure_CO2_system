#include "Server.hpp"

#include <iostream>

#include <boost/asio/connect.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include "JsonSerialization.hpp"
#include "SpdlogConfig.hpp"

static std::atomic<unsigned int> nextSessionId{1};

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;
using tcp       = net::ip::tcp;

Server::Server(boost::asio::io_context &io_context, const boost::asio::ip::tcp::endpoint &endpoint,
               const std::string &openWeatherApiKey, DoTaskCallback procDataCallback)
    : ioContext_(io_context), acceptor_(io_context, endpoint),
      openWeatherApiKey_(openWeatherApiKey), doTaskCallback_(procDataCallback)
{
    SPDLOG_INFO("Server starting on address {}, port {}", endpoint.address().to_string(),
                endpoint.port());
    startAccept();
}

std::string Server::fetchOutdoorCO2Level(const std::string &location)
{
    try {
        net::io_context ioc;
        tcp::resolver resolver{ioc};
        beast::tcp_stream stream{ioc};

        // OpenWeatherMap API endpoint
        auto const host   = "api.openweathermap.org";
        auto const port   = "80";
        auto const target = "/data/2.5/air_pollution?lat=35.6895&lon=139.6917&appid=" +
                            openWeatherApiKey_; // Use the provided location for real implementation

        auto const results = resolver.resolve(host, port);
        stream.connect(results);

        // Set up an HTTP GET request message
        http::request<http::string_body> req{http::verb::get, target, 11};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Send the HTTP request to the remote host
        http::write(stream, req);

        // This buffer is used for reading and must be persisted
        beast::flat_buffer buffer;

        // Declare a container to hold the response
        http::response<http::string_body> res;

        // Receive the HTTP response
        http::read(stream, buffer, res);

        // Gracefully close the socket
        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        if (ec && ec != beast::errc::not_connected)
            throw beast::system_error{ec};

        // Parse JSON response
        auto jsonResponse = nlohmann::json::parse(res.body());

        // Extract CO2 level (assuming it is in the 'components' object under 'co')
        auto co2Level = jsonResponse["list"][0]["components"]["co"];

        return co2Level.dump();
    } catch (const std::exception &e) {
        SPDLOG_ERROR("Error: {}", e.what());
        return "";
    }
}

void Server::startAccept()
{
    SPDLOG_TRACE("Server::startAccept");
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ioContext_);
    acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code &error) {
        handleAccept(error, socket);
    });
}

void Server::handleAccept(const boost::system::error_code &error,
                          std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    SPDLOG_TRACE("Server::handleAccept");
    if (!error) {
        SPDLOG_INFO("Connection accepted");
        unsigned int sessionId = nextSessionId++;
        auto session = std::make_shared<Session>(std::move(*socket), doTaskCallback_, sessionId);
        session->start();
        sessions[sessionId] = session;
    } else {
        SPDLOG_ERROR("Error on accept: {}", error.message());
    }
    startAccept();
}
