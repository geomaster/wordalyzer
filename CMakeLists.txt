cmake_minimum_required(VERSION 2.6)

# Enable debug symbols by default
# must be done before project() statement
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

project(wordalyzer)

include_directories("${PROJECT_BINARY_DIR}")

# Define sources and executable
add_executable("wordalyzer"
    src/main.cpp
    src/database.cpp
    src/audio.cpp
    src/common.cpp
    src/wav.cpp
    src/window.cpp
    src/endpointing.cpp
    )
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})

# Detect and add SFML
find_package(SFML 2 REQUIRED network audio graphics window system)
if(SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries("wordalyzer" ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
else(SFML_FOUND)
    message(FATAL_ERROR "SFML not found, but required.")
endif(SFML_FOUND)

# Detect and add SQLite
find_package(SQLite3 REQUIRED)
if(SQLITE3_FOUND)
    include_directories(${SQLITE3_INCLUDE_DIRS})
    target_link_libraries ("wordalyzer" ${SQLITE3_LIBRARIES})
else(SQLITE3_FOUND)
    message(FATAL_ERROR "SQLite not found, but required.")
endif(SQLITE3_FOUND)

# Detect and add Armadillo
find_package(Armadillo REQUIRED)
if(ARMADILLO_FOUND)
    include_directories(${ARMADILLO_INCLUDE_DIRS})
    target_link_libraries ("wordalyzer" ${ARMADILLO_LIBRARIES})
else(ARMADILLO_FOUND)
    message(FATAL_ERROR "Armadillo not found, but required.")
endif(ARMADILLO_FOUND)

# Install target
install(TARGETS "wordalyzer" DESTINATION bin)
