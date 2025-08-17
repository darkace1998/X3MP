cmake_minimum_required(VERSION 3.16)

project(X3MP VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard globally
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Options
option(BUILD_CLIENT "Build the C++ Client DLL" ON)
option(BUILD_SERVER "Build the C++ Server (legacy)" OFF)
option(BUILD_TESTS "Build unit tests" ON)

# Global compile features
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Add subdirectories
if(BUILD_CLIENT)
    add_subdirectory(Client)
endif()

if(BUILD_SERVER)
    add_subdirectory(Server)
endif()

# Testing
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Print build configuration
message(STATUS "X3MP Build Configuration:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build client: ${BUILD_CLIENT}")
message(STATUS "  Build server: ${BUILD_SERVER}")
message(STATUS "  Build tests: ${BUILD_TESTS}")