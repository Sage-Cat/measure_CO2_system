#include "SQLiteDatabase.hpp"

SQLiteDatabase::SQLiteDatabase(const char* dbPath) : db(nullptr) {
    if (sqlite3_open(dbPath, &db) != SQLITE_OK) {
        // some code...
    }
}

SQLiteDatabase::~SQLiteDatabase() {
    if (db) sqlite3_close(db);
}