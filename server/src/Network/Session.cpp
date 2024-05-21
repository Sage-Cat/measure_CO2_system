#include "Session.hpp"
#include "Data.hpp"
#include "JsonSerialization.hpp"
#include "SpdlogConfig.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
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

    auto req = std::make_shared<http::request<http::string_body>>();
    http::async_read(
        socket_, buffer_, *req,
        [this, self, req](beast::error_code ec, std::size_t bytes_transferred) {
            if (ec) {
                if (ec != http::error::end_of_stream) {
                    SPDLOG_ERROR("Session {}: Read error: {}", sessionId_, ec.message());
                }
                doClose();
                return;
            }

            try {
                RequestData requestData = deserializeRequest(req->body());

                doTaskCallback_(requestData, [this, self](ResponseData data) {
                    SPDLOG_TRACE("SendResponseCallback");
                    std::string response = serializeResponse(data);
                    doWrite(response);
                });

            } catch (const std::exception &e) {
                SPDLOG_ERROR("Session {}: Error processing data: {}", sessionId_, e.what());
                ResponseData errorData;
                errorData.error           = "Data processing error: " + std::string(e.what());
                std::string errorResponse = serializeResponse(errorData);
                doWrite(errorResponse);
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
    socket_.shutdown(tcp::socket::shutdown_send, ec);
    if (ec && ec != beast::errc::not_connected) {
        SPDLOG_ERROR("Session {}: Shutdown error: {}", sessionId_, ec.message());
    }
}

RequestData Session::deserializeRequest(const std::string &requestBody)
{
    auto json = nlohmann::json::parse(requestBody);
    SPDLOG_TRACE("Session::deserializeRequest | Got data from client: {}", json.dump());

    RequestData requestData;
    requestData.cmd = json.at(JsonKeys::CMD).get<std::string>();
    if (json.contains(JsonKeys::PARAMS)) {
        requestData.params = json.at(JsonKeys::PARAMS).get<std::vector<std::string>>();
    }
    return requestData;
}

std::string Session::serializeResponse(const ResponseData &responseData)
{
    nlohmann::json responseJson;

    if (!responseData.error.empty()) {
        responseJson[JsonKeys::ERR] = responseData.error;
    } else {
        responseJson = nlohmann::json::array();
        for (const auto &sample : responseData.measurements) {
            nlohmann::json j;
            nlohmann::adl_serializer<CO2Sample>::to_json(j, sample);
            responseJson.push_back(j);
        }
    }
    return responseJson.dump();
}
