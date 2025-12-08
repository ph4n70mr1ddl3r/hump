# Core unit tests

# deck_test
add_executable(deck_test deck_test.cpp)
target_link_libraries(deck_test gtest_main core common)
gtest_discover_tests(deck_test)

# hand_ranking_test
add_executable(hand_ranking_test hand_ranking_test.cpp)
target_link_libraries(hand_ranking_test gtest_main core common)
gtest_discover_tests(hand_ranking_test)

# betting_rules_test
add_executable(betting_rules_test betting_rules_test.cpp)
target_link_libraries(betting_rules_test gtest_main core common)
gtest_discover_tests(betting_rules_test)