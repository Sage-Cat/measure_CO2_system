find_package(SQLite3 REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(protocol_tests
    ProtocolTests.cpp
    ../src/Network/Protocol.cpp
)
target_include_directories(protocol_tests PRIVATE ../src ../src/Network)
target_link_libraries(protocol_tests PRIVATE nlohmann_json::nlohmann_json)
target_compile_options(protocol_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME protocol_tests COMMAND protocol_tests)

add_executable(database_tests
    DatabaseTests.cpp
    ../src/Database/SQLiteDatabase.cpp
)
target_include_directories(database_tests PRIVATE ../src ../src/Database)
target_link_libraries(database_tests PRIVATE SQLite::SQLite3 spdlog::spdlog)
target_compile_options(database_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME database_tests COMMAND database_tests)

add_executable(sensor_protocol_tests
    SensorProtocolTests.cpp
    ../src/Devices/CO2Sensor.cpp
)
target_include_directories(sensor_protocol_tests PRIVATE ../src ../src/Devices)
target_link_libraries(sensor_protocol_tests PRIVATE Boost::system spdlog::spdlog)
target_compile_options(sensor_protocol_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME sensor_protocol_tests COMMAND sensor_protocol_tests)
