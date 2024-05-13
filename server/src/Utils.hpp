#ifndef UTILS_HPP
#define UTILS_HPP

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

namespace Utils {
inline std::string getCurrentDateTime()
{
    auto now       = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm     = *std::localtime(&in_time_t);

    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

} // namespace Utils

#endif // UTILS_HPP
