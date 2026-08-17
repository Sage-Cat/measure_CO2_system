#include "Database/SQLiteDatabase.hpp"

#include <filesystem>
#include <stdexcept>

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
    const auto path = std::filesystem::temp_directory_path() / "measure-co2-system-test.sqlite3";
    std::filesystem::remove(path);
    {
        SQLiteDatabase database(path.string());
        database.addIndoorCO2Sample({"2024-05-01 12:00:02", "700"});
        database.addIndoorCO2Sample({"2024-05-01 12:00:01", "650"});
        const auto samples = database.getIndoorCO2Samples();
        require(samples.size() == 2, "unexpected sample count");
        require(samples.at(0).CO2Level == "650", "samples are not ordered by datetime");
        require(database.getIndoorCO2SamplesAfterDatetime("2024-05-01 12:00:01").size() == 1,
                "after-datetime query returned the wrong samples");
    }
    std::filesystem::remove(path);
}
