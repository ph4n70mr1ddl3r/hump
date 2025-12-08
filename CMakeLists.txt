cmake_minimum_required(VERSION 3.15)
project(PokerServer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTS "Build tests" ON)
option(BUILD_SERVER "Build server executable" ON)
option(BUILD_CLIENT "Build client executable" ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Find Boost
find_package(Boost 1.70 REQUIRED COMPONENTS system thread)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_definitions(-DBOOST_ALL_NO_LIB)
endif()

# nlohmann/json via FetchContent
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# Google Test via FetchContent (if tests enabled)
if(BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    FetchContent_MakeAvailable(googletest)
    include(GoogleTest)
endif()

# Add source subdirectories
add_subdirectory(src)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Installation (optional)
install(TARGETS poker_server poker_bot
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)