# Contract tests

# malformed_messages_test
add_executable(malformed_messages_test malformed_messages_test.cpp)
target_link_libraries(malformed_messages_test gtest_main core common Boost::system)
gtest_discover_tests(malformed_messages_test)

# welcome_test
add_executable(welcome_test welcome_test.cpp)
target_link_libraries(welcome_test gtest_main common core Boost::system)
gtest_discover_tests(welcome_test)

# hand_started_test
add_executable(hand_started_test hand_started_test.cpp)
target_link_libraries(hand_started_test gtest_main common core Boost::system)
gtest_discover_tests(hand_started_test)

# action_messages_test
add_executable(action_messages_test action_messages_test.cpp)
target_link_libraries(action_messages_test gtest_main common core Boost::system)
gtest_discover_tests(action_messages_test)

# hand_completed_test
add_executable(hand_completed_test hand_completed_test.cpp)
target_link_libraries(hand_completed_test gtest_main common core Boost::system)
gtest_discover_tests(hand_completed_test)