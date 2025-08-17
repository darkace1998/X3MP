cmake_minimum_required(VERSION 3.16)

project(X3MP_Client VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific configurations
if(WIN32)
    # Windows-specific settings
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
    set(PLATFORM_LIBS ws2_32 d3d9 d3dx9)
    set(PLATFORM_SOURCES )
else()
    # Linux/Unix-specific settings  
    set(PLATFORM_LIBS pthread dl)
    set(PLATFORM_SOURCES )
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../X3Net
)

# Source files
set(CLIENT_SOURCES
    Chatbox.cpp
    Console.cpp
    Logger.cpp
    Mod.cpp
    Renderer.cpp
    ReliabilityManager.cpp
    X3Functions.cpp
    X3Util.cpp
    client.cpp
    directx.cpp
    dllmain.cpp
    mem.cpp
    pugixml.cpp
    settings.cpp
)

# Header files (for IDE support)
set(CLIENT_HEADERS
    Chatbox.h
    Console.h
    Logger.h
    Mod.h
    Renderer.h
    ReliabilityManager.h
    X3Classes.h
    X3Functions.h
    X3Util.h
    astruct_9.h
    client.h
    defines.h
    directx.h
    mem.h
    pugiconfig.hpp
    pugixml.hpp
    settings.h
)

# Create the Client library
if(WIN32)
    add_library(Client SHARED ${CLIENT_SOURCES} ${CLIENT_HEADERS})
else()
    # On non-Windows, create a static library for testing purposes
    add_library(Client STATIC ${CLIENT_SOURCES} ${CLIENT_HEADERS})
endif()

# Link libraries
target_link_libraries(Client PRIVATE ${PLATFORM_LIBS})

# Compiler-specific options
if(MSVC)
    target_compile_options(Client PRIVATE /W4)
    # Disable specific MSVC warnings if needed
    target_compile_options(Client PRIVATE /wd26812) # Prefer 'enum class' over 'enum'
else()
    target_compile_options(Client PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Set output directory
set_target_properties(Client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# Install target
install(TARGETS Client
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)