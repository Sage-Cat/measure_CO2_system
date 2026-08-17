find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(SQLite3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED libgpiod)

find_path(XGBOOST_INCLUDE_DIR xgboost/c_api.h)
find_library(XGBOOST_LIB xgboost)
if(NOT XGBOOST_INCLUDE_DIR OR NOT XGBOOST_LIB)
    message(FATAL_ERROR "XGBoost headers and library are required")
endif()

include_directories(${Boost_INCLUDE_DIRS} ${SQLite3_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${XGBOOST_INCLUDE_DIR})

set(SOURCES
    Devices/CO2Sensor.cpp
    Devices/LED.cpp
    Database/SQLiteDatabase.cpp
    Network/Server.cpp
    Network/Session.cpp
    Network/Protocol.cpp
    Network/JsonSerialization.hpp
    Application.cpp
    main.cpp
)

add_executable(MeasuringServer ${SOURCES})

target_link_libraries(MeasuringServer 
    ${Boost_LIBRARIES} 
    ${SQLite3_LIBRARIES} 
    ${GPIOD_LIBRARIES}
    ${XGBOOST_LIB}
    OpenSSL::SSL
    OpenSSL::Crypto
    spdlog::spdlog 
    nlohmann_json::nlohmann_json
    Threads::Threads
)

target_include_directories(MeasuringServer PRIVATE ${GPIOD_INCLUDE_DIRS})
target_compile_options(MeasuringServer PRIVATE -Wall -Wextra -Wpedantic)

# Set the output directory for the executable
set_target_properties(MeasuringServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Database path
target_compile_definitions(MeasuringServer PRIVATE DATABASE_FILE_PATH="${CMAKE_BINARY_DIR}/bin/db.sl3")
# XGBoost model path
target_compile_definitions(MeasuringServer PRIVATE XGMODEL_FILE_PATH="${CMAKE_BINARY_DIR}/bin/cpp_model.model")
