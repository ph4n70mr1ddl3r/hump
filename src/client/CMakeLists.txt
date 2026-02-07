# Client library
add_library(client_lib STATIC
    client.cpp
    random_strategy.cpp
    delay.cpp
    stack_management.cpp
)

target_include_directories(client_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(client_lib PUBLIC core common)

# Client executable
add_executable(poker_bot main.cpp)
target_link_libraries(poker_bot PUBLIC client_lib core common Boost::system Boost::boost)