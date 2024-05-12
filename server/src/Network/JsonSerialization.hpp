#ifndef JSON_SERIALIZATION_HPP
#define JSON_SERIALIZATION_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "Data.hpp"

// Provide to/from_json functions for CO2Sample
namespace nlohmann {
template <> struct adl_serializer<CO2Sample> {
    static void to_json(json &j, const CO2Sample &sample)
    {
        j = json{{"datetime", sample.datetime}, {"CO2Level", sample.CO2Level}};
    }

    static void from_json(const json &j, CO2Sample &sample)
    {
        j.at("datetime").get_to(sample.datetime);
        j.at("CO2Level").get_to(sample.CO2Level);
    }
};
} // namespace nlohmann

#endif // JSON_SERIALIZATION_HPP
