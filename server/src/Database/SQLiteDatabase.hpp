#ifndef SQLITE_HPP
#define SQLITE_HPP

#include <sqlite3.h>

class SQLiteDatabase {
public:
    SQLiteDatabase(const char* dbPath);
    ~SQLiteDatabase();

private:
    sqlite3* db;
};

#endif