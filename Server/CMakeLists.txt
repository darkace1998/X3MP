cmake_minimum_required(VERSION 3.10)

project(X3MP_Server)

find_package (Threads REQUIRED)

find_package (Curses REQUIRED)

include_directories(../../SDKs/GameNetworkingSockets/include ../X3Net/ ${CURSES_INCLUDE_DIR} ../../SDKs/lua-5.4.3/src)

link_directories(../../SDKs/GameNetworkingSockets/bin ../../SDKs/lua-5.4.3/src)

add_executable(x3mp_server Screen.cpp Universe.cpp Script.cpp Server.cpp Config.cpp main.cpp)

target_link_libraries(x3mp_server PRIVATE ${CURSES_LIBRARIES} dl lua Threads::Threads GameNetworkingSockets.so)
