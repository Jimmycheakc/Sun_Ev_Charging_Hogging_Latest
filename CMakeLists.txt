cmake_minimum_required(VERSION 3.10)

project(ev_charging_hogging)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Add include directories
include_directories(/usr/local/include/Poco)

# Add link directories
link_directories(/usr/local/lib)

# List your source files
set(SOURCE_FILES
    ini_parser.cpp
    camera.cpp
    common.cpp
    log.cpp
    database.cpp
    central.cpp
    timer.cpp
    ev_charging.cpp
)

# Create an executable
add_executable(ev_charging ${SOURCE_FILES})

# Link against libraries
target_link_libraries(ev_charging PocoFoundation PocoNet PocoData PocoDataMySQL PocoUtil PocoJSON)
