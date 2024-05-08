#ifndef DATA_HPP
#define DATA_HPP

struct Data
{
    //some data from application
};

using Callback = std::function<void(std::string, Data)>;

#endif