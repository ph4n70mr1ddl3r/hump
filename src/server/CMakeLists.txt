# Server library
add_library(server_lib STATIC
    server.cpp
    table_manager.cpp
    player_action.cpp
    game_session.cpp
    connection_manager.cpp
    player_state.cpp
    websocket_session.cpp
)

target_include_directories(server_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(server_lib PUBLIC core common Boost::system Boost::boost)

# Server executable
add_executable(poker_server main.cpp)
target_link_libraries(poker_server PUBLIC server_lib)