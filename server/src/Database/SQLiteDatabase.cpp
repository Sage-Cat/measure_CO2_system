#include "SQLiteDatabase.hpp"

SQLiteDatabase::SQLiteDatabase(const char *dbPath) : db(nullptr)
{
    if (sqlite3_open(dbPath, &db) != SQLITE_OK) {
        // TODO: Create tables displaying the CO2 level data over time.
    }
}

SQLiteDatabase::~SQLiteDatabase()
{
    if (db)
        sqlite3_close(db);
}