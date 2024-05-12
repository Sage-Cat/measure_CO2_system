#include "SQLiteDatabase.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "SpdlogConfig.hpp"

SQLiteDatabase::SQLiteDatabase(const std::string &dbPath)
    : db(nullptr, &sqlite3_close), dbPath(dbPath)
{
    SPDLOG_TRACE("SQLiteDatabase::SQLiteDatabase - {}", dbPath);

    std::ifstream dbFile(dbPath);
    bool dbExists = dbFile.good();

    sqlite3 *tempDb = nullptr;
    if (sqlite3_open(dbPath.c_str(), &tempDb) != SQLITE_OK) {
        SPDLOG_ERROR("Error opening database: {}", sqlite3_errmsg(tempDb));
        throw std::runtime_error("Error opening database");
    }
    db.reset(tempDb);

    if (!dbExists) {
        SPDLOG_INFO("Database file created: {}", dbPath);
        initDatabaseSchema();
    } else {
        SPDLOG_INFO("Database opened: {}", dbPath);
    }
}

bool SQLiteDatabase::initDatabaseSchema()
{
    SPDLOG_TRACE("SQLiteDatabase::initDatabaseSchema");
    const char *sql = R"(
        CREATE TABLE IF NOT EXISTS Measurements (
            datetime DATETIME NOT NULL,
            CO2Level TEXT NOT NULL
        );
    )";

    char *errMsg = nullptr;
    if (sqlite3_exec(db.get(), sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        SPDLOG_ERROR("Error creating table: {}", errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    SPDLOG_INFO("Table 'Measurements' created.");
    return true;
}

bool SQLiteDatabase::addMeasurement(const CO2Sample &sample)
{
    SPDLOG_TRACE("SQLiteDatabase::addMeasurement");
    const std::string sql = "INSERT INTO Measurements (datetime, CO2Level) VALUES (?, ?);";
    sqlite3_stmt *stmt    = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        SPDLOG_ERROR("Error preparing insert statement: {}", sqlite3_errmsg(db.get()));
        return false;
    }
    std::unique_ptr<sqlite3_stmt, StatementDeleter> stmtUnique(stmt);

    sqlite3_bind_text(stmt, 1, sample.datetime.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, sample.CO2Level.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        SPDLOG_ERROR("Error inserting data: {}", sqlite3_errmsg(db.get()));
        return false;
    }

    SPDLOG_INFO("Measurement added: {} - CO2Level {}", sample.datetime, sample.CO2Level);
    return true;
}

std::vector<CO2Sample> SQLiteDatabase::getMeasurementsAfterDate(const std::string &dateAfter)
{
    SPDLOG_TRACE("SQLiteDatabase::getMeasurementsAfterDate | datetime - {}", dateAfter);
    const std::string sql = "SELECT datetime, CO2Level FROM Measurements WHERE datetime > ?";
    sqlite3_stmt *stmt    = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        SPDLOG_ERROR("Error preparing select statement: {}", sqlite3_errmsg(db.get()));
        return {};
    }
    std::unique_ptr<sqlite3_stmt, StatementDeleter> stmtUnique(stmt);

    sqlite3_bind_text(stmt, 1, dateAfter.c_str(), -1, SQLITE_TRANSIENT);

    std::vector<CO2Sample> measurements;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        measurements.emplace_back(
            CO2Sample{.datetime = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)),
                      .CO2Level = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1))});
    }

    return measurements;
}
