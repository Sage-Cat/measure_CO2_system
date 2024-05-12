cmake_minimum_required(VERSION 3.10) # ensure you specify the right version
project(MeasuringServer)

find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(SQLite3 REQUIRED)
find_package(spdlog REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} ${SQLite3_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
    Application/Application.cpp
    CO2Sensor/CO2Sensor.cpp
    Database/SQLiteDatabase.cpp
    Network/Server.cpp
    Network/Session.cpp
    main.cpp
)

add_executable(MeasuringServer ${SOURCES})

target_link_libraries(MeasuringServer ${Boost_LIBRARIES} ${SQLite3_LIBRARIES} spdlog::spdlog)

# Set the output directory for the executable
set_target_properties(MeasuringServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Database path
target_compile_definitions(MeasuringServer PRIVATE DATABASE_FILE_PATH="${CMAKE_BINARY_DIR}/bin/db.sl3")
