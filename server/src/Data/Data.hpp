#ifndef DATA_HPP
#define DATA_HPP

#include <functional>
#include <string>

struct Data {
  // some data from application
};

using ProcessDataCallback = std::function<void(std::string, Data)>;

#endif