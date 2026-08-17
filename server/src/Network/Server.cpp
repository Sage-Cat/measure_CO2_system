#include "Server.hpp"

#include <iostream>

#include <boost/asio/connect.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <openssl/ssl.h>

#include "JsonSerialization.hpp"
#include "SpdlogConfig.hpp"

static std::atomic<unsigned int> nextSessionId{1};

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;
namespace ssl   = net::ssl;
using tcp       = net::ip::tcp;

Server::Server(boost::asio::io_context &io_context, const boost::asio::ip::tcp::endpoint &endpoint,
               const std::string &openWeatherApiKey, DoTaskCallback procDataCallback)
    : ioContext_(io_context), acceptor_(io_context, endpoint),
      doTaskCallback_(std::move(procDataCallback)), openWeatherApiKey_(openWeatherApiKey)
{
    SPDLOG_INFO("Server starting on address {}, port {}", endpoint.address().to_string(),
                endpoint.port());
    startAccept();
}

std::string Server::fetchOutdoorCOLevel(const std::string & /*location*/)
{
    try {
        net::io_context ioc;
        tcp::resolver resolver{ioc};
        ssl::context sslContext{ssl::context::tls_client};
        sslContext.set_default_verify_paths();
        beast::ssl_stream<beast::tcp_stream> stream{ioc, sslContext};

        // OpenWeatherMap API endpoint
        auto const host   = "api.openweathermap.org";
        auto const port   = "443";
        auto const target = "/data/2.5/air_pollution?lat=35.6895&lon=139.6917&appid=" +
                            openWeatherApiKey_; // Use the provided location for real implementation

        auto const results = resolver.resolve(host, port);
        if (!SSL_set_tlsext_host_name(stream.native_handle(), host)) {
            throw beast::system_error(beast::error_code(static_cast<int>(::ERR_get_error()),
                                                        net::error::get_ssl_category()));
        }
        stream.set_verify_mode(ssl::verify_peer);
        stream.set_verify_callback(ssl::host_name_verification(host));
        beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(10));
        beast::get_lowest_layer(stream).connect(results);
        stream.handshake(ssl::stream_base::client);

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
        stream.shutdown(ec);
        if (ec == net::error::eof || ec == ssl::error::stream_truncated) {
            ec = {};
        }
        if (ec) {
            throw beast::system_error{ec};
        }

        if (res.result() != http::status::ok) {
            throw std::runtime_error("OpenWeather returned HTTP " +
                                     std::to_string(res.result_int()));
        }

        // Parse JSON response
        auto jsonResponse = nlohmann::json::parse(res.body());

        // OpenWeather exposes carbon monoxide here, in micrograms per cubic metre.
        auto co2Level = jsonResponse.at("list").at(0).at("components").at("co");

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
    } else {
        SPDLOG_ERROR("Error on accept: {}", error.message());
    }
    startAccept();
}
