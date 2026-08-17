#include "Network/Protocol.hpp"

#include <stdexcept>
#include <string>

namespace {
void require(bool condition, const char *message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}
} // namespace

int main()
{
    const auto request = Protocol::deserializeRequest(
        R"({"cmd":"get_indoor_after","params":["2024-05-01 12:00:00"]})");
    require(request.cmd == "get_indoor_after", "cmd did not round-trip");
    require(request.params.size() == 1, "params did not round-trip");

    bool rejected = false;
    try {
        Protocol::deserializeRequest(R"({"cmd":7})");
    } catch (const std::exception &) {
        rejected = true;
    }
    require(rejected, "non-string command was accepted");

    rejected = false;
    try {
        Protocol::deserializeRequest(std::string(Protocol::MAX_BODY_BYTES + 1, 'x'));
    } catch (const std::invalid_argument &) {
        rejected = true;
    }
    require(rejected, "oversized body was accepted");

    ResponseData response{{CO2Sample{"2024-05-01 12:00:00", "650"}}, {}};
    require(Protocol::serializeResponse(response) ==
                R"([{"CO2Level":"650","datetime":"2024-05-01 12:00:00"}])",
            "response serialization changed");
}
