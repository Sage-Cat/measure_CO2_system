#ifndef SQLITE_DATABASE_HPP
#define SQLITE_DATABASE_HPP

#include <memory>

#include <boost/noncopyable.hpp>
#include <sqlite3.h>

#include "Data.hpp"

class SQLiteDatabase : private boost::noncopyable {
public:
    explicit SQLiteDatabase(const std::string &dbPath);
    ~SQLiteDatabase() = default;

    bool initDatabaseSchema();
    bool addMeasurement(const CO2Sample &sample);
    std::vector<CO2Sample> getMeasurementsAfterDate(const std::string &dateAfter);

private:
    struct StatementDeleter {
        void operator()(sqlite3_stmt *stmt) const { sqlite3_finalize(stmt); }
    };

private:
    std::unique_ptr<sqlite3, decltype(&sqlite3_close)> db;
    std::string dbPath;
};

#endif // SQLITE_DATABASE_HPP
