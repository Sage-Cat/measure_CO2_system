#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include "Data.hpp"

#include <cstddef>
#include <string>
#include <string_view>

namespace Protocol {
inline constexpr std::size_t MAX_BODY_BYTES = 8 * 1024;

RequestData deserializeRequest(std::string_view body);
std::string serializeResponse(const ResponseData &responseData);
} // namespace Protocol

#endif
