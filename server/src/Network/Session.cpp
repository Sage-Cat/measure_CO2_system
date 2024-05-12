#include "Session.hpp"

#include "JsonSerialization.hpp"

#include "SpdlogConfig.hpp"

Session::Session(io_context &io_context, tcp::socket &&socket, DoTaskCallback doTaskCallback,
                 unsigned int sessionId)
    : socket_(std::move(socket)), doTaskCallback_(doTaskCallback), sessionId_(sessionId),
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
            try {
                auto json = nlohmann::json::parse(std::string(data_.data(), length));
                RequestData requestData{.cmd    = json.at("cmd").get<std::string>(),
                                        .param1 = json.at("param1").get<std::string>()};
                doTaskCallback_(requestData, [this](ResponseData data) {
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
                    {"error", std::string("Invalid JSON format: ") + parseErr.what()}};
                doWrite(errorJson.dump());
            } catch (const nlohmann::json::out_of_range &oor) {
                SPDLOG_ERROR("Session {}: Missing required JSON keys: {}", sessionId_, oor.what());
                nlohmann::json errorJson = {
                    {"error", std::string("Missing required data: ") + oor.what()}};
                doWrite(errorJson.dump());
            } catch (const std::exception &e) {
                SPDLOG_ERROR("Session {}: Error processing data: {}", sessionId_, e.what());
                nlohmann::json errorJson = {
                    {"error", std::string("Data processing error: ") + e.what()}};
                doWrite(errorJson.dump());
            }
        } else {
            SPDLOG_ERROR("Session {}: Read error: {}", sessionId_, ec.message());
        }
    });
}

void Session::doWrite(const std::string &response)
{
    SPDLOG_TRACE("Session::doWrite");
    auto self(shared_from_this());
    std::copy(response.begin(), response.end(), data_.begin());
    async_write(socket_, buffer(data_, response.size()),
                [this, self](boost::system::error_code ec, std::size_t) {
                    if (!ec) {
                        SPDLOG_INFO("Session {}: Data written successfully", sessionId_);
                        doRead();
                    } else {
                        SPDLOG_ERROR("Session {}: Write error: {}", sessionId_, ec.message());
                    }
                });
}
