#include "Session.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>

#include "JsonSerialization.hpp"
#include "SpdlogConfig.hpp"

namespace beast = boost::beast;
namespace http  = beast::http;
using tcp       = boost::asio::ip::tcp;

Session::Session(tcp::socket &&socket, DoTaskCallback doTaskCallback, unsigned int sessionId)
    : socket_(std::move(socket)), doTaskCallback_(doTaskCallback), sessionId_(sessionId)
{
    SPDLOG_INFO("Session {} created", sessionId_);
}

void Session::start() { doRead(); }

unsigned int Session::getId() const { return sessionId_; }

void Session::doRead()
{
    SPDLOG_TRACE("Session::doRead");
    auto self = shared_from_this();

    // Read an HTTP request
    auto req = std::make_shared<http::request<http::string_body>>();
    http::async_read(
        socket_, buffer_, *req,
        [this, self, req](beast::error_code ec, std::size_t bytes_transferred) {
            if (ec) {
                SPDLOG_ERROR("Session {}: Read error: {}", sessionId_, ec.message());
                return;
            }

            try {
                auto json = nlohmann::json::parse(req->body());
                SPDLOG_TRACE("Session::doRead | Got data from client: {}", json.dump());

                RequestData requestData{.cmd    = json.at("cmd").get<std::string>(),
                                        .param1 = json.at("param1").get<std::string>()};

                doTaskCallback_(requestData, [this, self](ResponseData data) {
                    SPDLOG_TRACE("SendResponseCallback");
                    nlohmann::json responseJson = nlohmann::json::array();
                    for (const auto &sample : data.measurements) {
                        nlohmann::json j;
                        nlohmann::adl_serializer<CO2Sample>::to_json(j, sample);
                        responseJson.push_back(j);
                    }
                    doWrite(responseJson.dump());
                });

            } catch (const nlohmann::json::parse_error &parseErr) {
                SPDLOG_ERROR("Session {}: JSON parsing error: {}", sessionId_, parseErr.what());
                nlohmann::json errorJson = {
                    {"error", "Invalid JSON format: " + std::string(parseErr.what())}};
                doWrite(errorJson.dump());
            } catch (const nlohmann::json::out_of_range &oor) {
                SPDLOG_ERROR("Session {}: Missing required JSON keys: {}", sessionId_, oor.what());
                nlohmann::json errorJson = {
                    {"error", "Missing required data: " + std::string(oor.what())}};
                doWrite(errorJson.dump());
            } catch (const std::exception &e) {
                SPDLOG_ERROR("Session {}: Error processing data: {}", sessionId_, e.what());
                nlohmann::json errorJson = {
                    {"error", "Data processing error: " + std::string(e.what())}};
                doWrite(errorJson.dump());
            }
        });
}

void Session::doWrite(const std::string &response)
{
    SPDLOG_TRACE("Session::doWrite | Sending to client: {}", response);
    auto self = shared_from_this();

    auto res = std::make_shared<http::response<http::string_body>>(http::status::ok, 11);
    res->set(http::field::server, "Boost.Beast");
    res->set(http::field::content_type, "application/json");
    res->body() = response;
    res->prepare_payload();

    http::async_write(socket_, *res, [this, self, res](beast::error_code ec, std::size_t) {
        if (!ec) {
            SPDLOG_INFO("Session {}: Data written successfully", sessionId_);
            doRead();
        } else {
            SPDLOG_ERROR("Session {}: Write error: {}", sessionId_, ec.message());
        }
    });
}