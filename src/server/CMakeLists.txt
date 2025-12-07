# Server executable
add_executable(poker_server
    server.cpp
    table_manager.cpp
    player_action.cpp
    game_session.cpp
    connection_manager.cpp
    player_state.cpp
    player_removal.cpp
    websocket_session.cpp
    main.cpp
)

target_include_directories(poker_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(poker_server PUBLIC core common Boost::system Boost::boost)