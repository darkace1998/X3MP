cmake_minimum_required(VERSION 3.16)

# Find Google Test
find_package(GTest QUIET)

if(NOT GTest_FOUND)
    # If Google Test is not found, try to fetch it
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9fd7188aabd5768f7ee2885.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# Test executable
add_executable(
    x3mp_tests
    test_logger.cpp
    test_reliability.cpp
    test_packet_serialization.cpp
    # Add the implementation files that need testing
    ../Client/Logger.cpp
    ../Client/ReliabilityManager.cpp
)

# Include directories for tests
target_include_directories(x3mp_tests PRIVATE
    ../Client
    ../X3Net
)

# Link against gtest
if(GTest_FOUND)
    target_link_libraries(x3mp_tests gtest_main gtest)
else()
    target_link_libraries(x3mp_tests gtest_main gtest)
endif()

# Platform-specific libraries for tests
if(WIN32)
    target_link_libraries(x3mp_tests ws2_32)
else()
    target_link_libraries(x3mp_tests pthread)
endif()

# Enable testing
include(GoogleTest)
gtest_discover_tests(x3mp_tests)