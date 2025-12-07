# Tests CMakeLists.txt
enable_testing()

# Unit tests
add_subdirectory(unit)

# Integration tests
add_subdirectory(integration)

# Contract tests
add_subdirectory(contract)