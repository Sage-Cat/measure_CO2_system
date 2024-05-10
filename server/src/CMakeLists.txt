find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(SQLite3 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS} ${SQLite3_INCLUDE_DIRS} )
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Data)

set(SOURCES
    Application/Application.cpp
    CO2Sensor/CO2Sensor.cpp
    Database/SQLiteDatabase.cpp
    Server.cpp
    Session.cpp
    main.cpp
)

add_executable(MeasuringServer ${SOURCES})

target_link_libraries(MeasuringServer ${Boost_LIBRARIES} ${SQLite3_LIBRARIES})
