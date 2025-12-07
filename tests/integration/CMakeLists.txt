# Integration tests

# websocket_connection_test
add_executable(websocket_connection_test websocket_connection_test.cpp)
target_link_libraries(websocket_connection_test gtest_main core common Boost::system Boost::thread)
gtest_discover_tests(websocket_connection_test)

# full_hand_test
add_executable(full_hand_test full_hand_test.cpp)
target_link_libraries(full_hand_test gtest_main core common Boost::system Boost::thread)
gtest_discover_tests(full_hand_test)