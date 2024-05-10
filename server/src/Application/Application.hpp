#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "Data.hpp"
#include <functional>
#include <sqlite3.h>

class CO2Sensor;
class SQLiteDatabase;

class Application {

public:
  using Callback = std::function<void(Data)>;

  Application(CO2Sensor &co2Sensor, SQLiteDatabase &sqliteDatabase);
  ~Application();

  void doTask(Callback callback);

private:
  CO2Sensor &sensor;
  SQLiteDatabase &database;
};

#endif