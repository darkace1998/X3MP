cmake_minimum_required(VERSION 3.16)

# Find Google Test
find_package(GTest QUIET)

if(NOT GTest_FOUND)
    # Try to fetch Google Test (only if network is available)
    include(FetchContent)
    set(FETCHCONTENT_QUIET FALSE)
    
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9fd7188aabd5768f7ee2885.zip
        DOWNLOAD_EXTRACT_TIMESTAMP true
    )
    
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    
    # Try to make googletest available, but continue if it fails
    FetchContent_MakeAvailable(googletest)
    set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
    
    # Check if the fetch was successful
    if(NOT TARGET gtest_main)
        message(WARNING "Google Test could not be downloaded. Skipping test build.")
        return()
    endif()
endif()

# Only proceed if we have Google Test available
if(TARGET gtest_main OR GTest_FOUND)
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
    
    message(STATUS "Tests configured successfully")
else()
    message(STATUS "Google Test not available - tests will be skipped")
endif()