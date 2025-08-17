# X3: Multiplayer (X3MP)

X3MP is an experimental, open-source multiplayer mod for the game **X3: Albion Prelude**.

## Project Status

**Warning:** This mod is in a very early stage of development. It is not stable, feature-complete, or suitable for a normal gameplay experience. Expect bugs, crashes, and missing features.

## Recent Improvements

This version includes several modernization updates:

- **Cross-Platform Build System**: CMake support and automated build scripts for Linux, macOS, and Windows
- **Reliability Protocol**: UDP communication enhanced with sequence numbers, acknowledgments, and retransmission
- **Modern C++**: Raw pointers replaced with smart pointers and RAII-based resource management
- **Centralized Logging**: Runtime-configurable logging system for both Go and C++ components
- **Comprehensive Testing**: Unit tests for packet serialization, reliability protocol, and core logic

## Quick Start

### Building

**Automated build (recommended):**
```bash
# Linux/macOS
./build.sh

# Windows
build.bat
```

**Manual build:**
See [BUILD.md](BUILD.md) for detailed build instructions and requirements.

### Running

1. Build the Go server: `cd GoServer && go build -o bin/x3mp_goserver .`
2. Start the server: `./bin/x3mp_goserver`
3. Build the C++ client using the build scripts or CMake
4. Use the X3MP_Launcher to inject the client DLL into X3: Albion Prelude

## Architecture

The mod uses a client-server architecture.
-   The **Client** is a DLL that is injected into the game to add multiplayer functionality.
-   The **Server** is a dedicated, standalone application that manages game state and synchronizes clients.

### The Go Server

The dedicated server is written in pure Go and is located in the `GoServer/` directory. It uses a UDP-based protocol with reliability features for communication.

For instructions on how to build and run the server, see the [GoServer/README.md](GoServer/README.md) file.

### The C++ Client

The client is a C++ DLL that hooks into the X3 game engine. It is located in the `Client/` directory.

For instructions on how to build the client, see the [Client/README.md](Client/README.md) file.

## Technical Features

### Reliability Protocol
- Sequence numbers and acknowledgments for UDP packets
- Automatic retransmission with configurable timeouts
- Duplicate packet filtering and ordered delivery
- Configurable reliability parameters

### Logging System
- Centralized logging interface for both C++ and Go components
- Runtime-configurable log levels (Debug, Info, Warning, Error)
- Thread-safe implementation with timestamp formatting
- Consistent output formatting across all components

### Modern C++ Design
- Smart pointers (`std::unique_ptr`, `std::shared_ptr`) replace raw pointers
- RAII-based resource management for network sockets and game objects
- Exception-safe code patterns
- C++17 standard compliance

### Cross-Platform Support
- CMake build system for Windows, Linux, and macOS
- Automated build scripts with dependency detection
- Platform-specific optimizations and configurations
- Comprehensive testing on multiple platforms

## How to Use

For more information on the project's history, installation, and community discussion, please visit the official thread on the Egosoft forums:
[https://forum.egosoft.com/viewtopic.php?f=94&t=436927](https://forum.egosoft.com/viewtopic.php?f=94&t=436927)
