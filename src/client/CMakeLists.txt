# Client executable
add_executable(poker_bot
    main.cpp
    client.cpp
    bot.cpp
    random_strategy.cpp
    delay.cpp
    stack_management.cpp
)

target_include_directories(poker_bot PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(poker_bot PUBLIC core common Boost::system Boost::boost)