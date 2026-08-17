#include "Protocol.hpp"

#include "JsonSerialization.hpp"

#include <nlohmann/json.hpp>
#include <stdexcept>

namespace Protocol {
RequestData deserializeRequest(std::string_view body)
{
    if (body.size() > MAX_BODY_BYTES) {
        throw std::invalid_argument("request body is too large");
    }

    const auto json = nlohmann::json::parse(body);
    if (!json.is_object()) {
        throw std::invalid_argument("request must be a JSON object");
    }

    RequestData request;
    request.cmd = json.at(JsonKeys::CMD).get<std::string>();
    if (request.cmd.empty() || request.cmd.size() > 64) {
        throw std::invalid_argument("cmd must contain 1 to 64 characters");
    }
    if (json.contains(JsonKeys::PARAMS)) {
        request.params = json.at(JsonKeys::PARAMS).get<std::vector<std::string>>();
    }
    if (request.params.size() > 8) {
        throw std::invalid_argument("too many params");
    }
    for (const auto &param : request.params) {
        if (param.size() > 128) {
            throw std::invalid_argument("param is too long");
        }
    }
    return request;
}

std::string serializeResponse(const ResponseData &responseData)
{
    nlohmann::json responseJson;
    if (!responseData.error.empty()) {
        responseJson[JsonKeys::ERR] = responseData.error;
    } else {
        responseJson = responseData.measurements;
    }
    return responseJson.dump();
}
} // namespace Protocol
