#include "SQLiteDatabase.hpp"
#include "SpdlogConfig.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>

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
        CREATE TABLE IF NOT EXISTS IndoorCO2 (
            datetime DATETIME NOT NULL,
            CO2Level TEXT NOT NULL
        );
        CREATE TABLE IF NOT EXISTS OutdoorCO2 (
            datetime DATETIME NOT NULL,
            CO2Level TEXT NOT NULL
        );
    )";

    char *errMsg = nullptr;
    if (sqlite3_exec(db.get(), sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        SPDLOG_ERROR("Error creating tables: {}", errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    SPDLOG_INFO("Tables 'IndoorCO2' and 'OutdoorCO2' created.");
    return true;
}

bool SQLiteDatabase::addIndoorCO2Sample(const CO2Sample &sample)
{
    SPDLOG_TRACE("SQLiteDatabase::addIndoorCO2Sample");
    const std::string sql = "INSERT INTO IndoorCO2 (datetime, CO2Level) VALUES (?, ?);";
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

    SPDLOG_INFO("Indoor CO2 sample added: {} - CO2Level {}", sample.datetime, sample.CO2Level);
    return true;
}

bool SQLiteDatabase::addOutdoorCO2Sample(const CO2Sample &sample)
{
    SPDLOG_TRACE("SQLiteDatabase::addOutdoorCO2Sample");
    const std::string sql = "INSERT INTO OutdoorCO2 (datetime, CO2Level) VALUES (?, ?);";
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

    SPDLOG_INFO("Outdoor CO2 sample added: {} - CO2Level {}", sample.datetime, sample.CO2Level);
    return true;
}

std::vector<CO2Sample> SQLiteDatabase::getIndoorCO2Samples()
{
    SPDLOG_TRACE("SQLiteDatabase::getIndoorCO2Samples");
    const std::string sql = "SELECT datetime, CO2Level FROM IndoorCO2;";
    sqlite3_stmt *stmt    = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        SPDLOG_ERROR("Error preparing select statement: {}", sqlite3_errmsg(db.get()));
        return {};
    }
    std::unique_ptr<sqlite3_stmt, StatementDeleter> stmtUnique(stmt);

    std::vector<CO2Sample> measurements;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        measurements.emplace_back(
            CO2Sample{.datetime = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)),
                      .CO2Level = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1))});
    }

    return measurements;
}

std::vector<CO2Sample> SQLiteDatabase::getOutdoorCO2Samples()
{
    SPDLOG_TRACE("SQLiteDatabase::getOutdoorCO2Samples");
    const std::string sql = "SELECT datetime, CO2Level FROM OutdoorCO2;";
    sqlite3_stmt *stmt    = nullptr;
    if (sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        SPDLOG_ERROR("Error preparing select statement: {}", sqlite3_errmsg(db.get()));
        return {};
    }
    std::unique_ptr<sqlite3_stmt, StatementDeleter> stmtUnique(stmt);

    std::vector<CO2Sample> measurements;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        measurements.emplace_back(
            CO2Sample{.datetime = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)),
                      .CO2Level = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1))});
    }

    return measurements;
}

std::vector<CO2Sample> SQLiteDatabase::getIndoorCO2SamplesAfterDatetime(const std::string &dateAfter)
{
    SPDLOG_TRACE("SQLiteDatabase::getIndoorCO2SamplesAfterDatetime | datetime - {}", dateAfter);
    const std::string sql = "SELECT datetime, CO2Level FROM IndoorCO2 WHERE datetime > ?;";
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
