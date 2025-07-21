cmake_minimum_required(VERSION 3.16)
project(X3MP_Client)

# Set up cross-compilation for Windows
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/../Client
    ${CMAKE_SOURCE_DIR}/../X3Net
)

# X3Net static library
add_library(X3Net STATIC
    ../X3Net/net_entity.h
    ../X3Net/net_message.h
    ../X3Net/net_packets.h
)
set_target_properties(X3Net PROPERTIES LINKER_LANGUAGE CXX)

# Define source files (excluding DirectX-dependent files for now)
set(CLIENT_SOURCES
    ../Client/client.cpp
    ../Client/client.h
    ../Client/Console.cpp
    ../Client/Console.h
    ../Client/Chatbox.cpp
    ../Client/Chatbox.h
    ../Client/settings.cpp
    ../Client/settings.h
    ../Client/mem.cpp
    ../Client/mem.h
    ../Client/pugixml.cpp
    ../Client/pugixml.hpp
    ../Client/pugiconfig.hpp
    ../Client/X3Functions.cpp
    ../Client/X3Functions.h
    ../Client/X3Util.cpp
    ../Client/X3Util.h
    ../Client/X3Classes.h
    ../Client/defines.h
    ../Client/astruct_9.h
    # NOTE: Excluding DirectX and DLL injection files:
    # ../Client/Renderer.cpp - requires DirectX
    # ../Client/directx.cpp - requires DirectX
    # ../Client/dllmain.cpp - requires DLL injection and DirectX
)

# Create executable (not DLL for cross-compilation test)
add_executable(x3mp_client ${CLIENT_SOURCES})

# Link with X3Net
target_link_libraries(x3mp_client X3Net)

# Windows-specific libraries (basic ones)
target_link_libraries(x3mp_client
    -lws2_32
    -lpthread
    -static-libgcc
    -static-libstdc++
)

# Compiler flags
target_compile_definitions(x3mp_client PRIVATE
    -DWIN32
    -D_WIN32
    -DUNICODE
    -D_UNICODE
    -DNDEBUG
)

# Disable some warnings for cross-compilation
target_compile_options(x3mp_client PRIVATE
    -Wall
    -Wno-unused-variable
    -Wno-unused-function
    -O2
)