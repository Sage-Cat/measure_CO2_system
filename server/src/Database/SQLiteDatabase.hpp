#ifndef SQLITE_DATABASE_HPP
#define SQLITE_DATABASE_HPP

#include "Data.hpp"
#include <boost/noncopyable.hpp>
#include <memory>
#include <sqlite3.h>
#include <vector>

class SQLiteDatabase : private boost::noncopyable {
public:
    explicit SQLiteDatabase(const std::string &dbPath);
    ~SQLiteDatabase() = default;

    bool initDatabaseSchema();
    bool addIndoorCO2Sample(const CO2Sample &sample);
    bool addOutdoorCO2Sample(const CO2Sample &sample);
    std::vector<CO2Sample> getIndoorCO2Samples();
    std::vector<CO2Sample> getOutdoorCO2Samples();
    std::vector<CO2Sample> getIndoorCO2SamplesAfterDatetime(const std::string &dateAfter);

private:
    struct StatementDeleter {
        void operator()(sqlite3_stmt *stmt) const { sqlite3_finalize(stmt); }
    };

private:
    std::unique_ptr<sqlite3, decltype(&sqlite3_close)> db;
    std::string dbPath;
};

#endif // SQLITE_DATABASE_HPP
