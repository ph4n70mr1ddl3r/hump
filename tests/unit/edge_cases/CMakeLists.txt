# Edge cases unit tests

# edge_cases_test
add_executable(edge_cases_test edge_cases_test.cpp)
target_link_libraries(edge_cases_test gtest_main core common server client)
gtest_discover_tests(edge_cases_test)