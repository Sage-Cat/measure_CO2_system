#include "Session.hpp"
#include "Data.hpp"
#include "Protocol.hpp"
#include "SpdlogConfig.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <chrono>
#include <nlohmann/json.hpp>

namespace beast = boost::beast;
namespace http  = beast::http;
using tcp       = boost::asio::ip::tcp;

Session::Session(tcp::socket socket, DoTaskCallback doTaskCallback, unsigned int sessionId)
    : socket_(std::move(socket)), doTaskCallback_(std::move(doTaskCallback)), sessionId_(sessionId)
{
    SPDLOG_INFO("Session {} created", sessionId_);
}

void Session::start() { doRead(); }

unsigned int Session::getId() const { return sessionId_; }

void Session::doRead()
{
    SPDLOG_TRACE("Session::doRead");
    auto self = shared_from_this();

    auto parser = std::make_shared<http::request_parser<http::string_body>>();
    parser->body_limit(Protocol::MAX_BODY_BYTES);
    socket_.expires_after(std::chrono::seconds(10));
    http::async_read(
        socket_, buffer_, *parser, [this, self, parser](beast::error_code ec, std::size_t) {
            if (ec) {
                if (ec == http::error::body_limit) {
                    doWrite(http::status::payload_too_large,
                            R"({"error":"request body is too large"})");
                    return;
                }
                if (ec != http::error::end_of_stream) {
                    SPDLOG_ERROR("Session {}: Read error: {}", sessionId_, ec.message());
                }
                doClose();
                return;
            }

            try {
                const auto req = parser->release();
                if (req.method() != http::verb::post) {
                    doWrite(http::status::method_not_allowed, R"({"error":"POST required"})");
                    return;
                }
                if (req.target() != "/api/co2sensor") {
                    doWrite(http::status::not_found, R"({"error":"not found"})");
                    return;
                }
                const auto contentType = req[http::field::content_type];
                if (!contentType.starts_with("application/json")) {
                    doWrite(http::status::unsupported_media_type,
                            R"({"error":"application/json required"})");
                    return;
                }
                RequestData requestData = Protocol::deserializeRequest(req.body());

                doTaskCallback_(requestData, [this, self](ResponseData data) {
                    SPDLOG_TRACE("SendResponseCallback");
                    doWrite(http::status::ok, Protocol::serializeResponse(data));
                });

            } catch (const std::exception &e) {
                SPDLOG_WARN("Session {}: Invalid request: {}", sessionId_, e.what());
                doWrite(http::status::bad_request, R"({"error":"invalid request"})");
            }
        });
}

void Session::doWrite(http::status status, const std::string &response)
{
    SPDLOG_TRACE("Session::doWrite");
    auto self = shared_from_this();

    auto res = std::make_shared<http::response<http::string_body>>(status, 11);
    res->set(http::field::server, "Boost.Beast");
    res->set(http::field::content_type, "application/json");
    res->body() = response;
    res->prepare_payload();

    socket_.expires_after(std::chrono::seconds(10));
    http::async_write(socket_, *res, [this, self, res](beast::error_code ec, std::size_t) {
        if (!ec) {
            SPDLOG_INFO("Session {}: Data written successfully", sessionId_);
            doClose();
        } else {
            SPDLOG_ERROR("Session {}: Write error: {}", sessionId_, ec.message());
        }
    });
}

void Session::doClose()
{
    SPDLOG_TRACE("Session::doClose");
    beast::error_code ec;
    socket_.socket().shutdown(tcp::socket::shutdown_send, ec);
    if (ec && ec != beast::errc::not_connected) {
        SPDLOG_ERROR("Session {}: Shutdown error: {}", sessionId_, ec.message());
    }
}
