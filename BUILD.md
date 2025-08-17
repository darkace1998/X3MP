# Building X3MP

X3MP now supports cross-platform building using CMake and includes both automated build scripts and manual build instructions.

## Quick Start

### Automated Build (Recommended)

**Linux/macOS:**
```bash
# Build everything
./build.sh

# Build only Go server
./build.sh --go-only

# Build only C++ client
./build.sh --cpp-only

# Install dependencies and build
./build.sh --install-deps

# Debug build with 8 parallel jobs
./build.sh --type Debug --jobs 8
```

**Windows:**
```cmd
# Build everything
build.bat

# Build only Go server
build.bat --go-only

# Build only C++ client  
build.bat --cpp-only

# Debug build
build.bat --type Debug
```

### Manual Build

#### Prerequisites

**All Platforms:**
- CMake 3.16 or later
- C++17 compatible compiler (GCC 7+, Clang 7+, MSVC 2019+)
- Go 1.19 or later (for Go server)

**Linux:**
```bash
# Ubuntu/Debian
sudo apt-get install cmake build-essential libgtest-dev

# CentOS/RHEL
sudo yum install cmake gcc-c++ gtest-devel

# Arch Linux
sudo pacman -S cmake gcc gtest
```

**macOS:**
```bash
# Using Homebrew
brew install cmake googletest
```

**Windows:**
- Visual Studio 2019+ with C++ workload, OR
- MinGW-w64 with GCC 7+
- vcpkg (recommended for dependency management)

#### Building the Go Server

```bash
cd GoServer
go mod tidy
go test ./...
go build -o bin/x3mp_goserver .
```

#### Building the C++ Client

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON

# Build
cmake --build . --config Release -j4

# Run tests
ctest --output-on-failure -C Release
```

## Build Options

### CMake Options

- `BUILD_CLIENT=ON/OFF` - Build the C++ client (default: ON)
- `BUILD_SERVER=ON/OFF` - Build the legacy C++ server (default: OFF)  
- `BUILD_TESTS=ON/OFF` - Build unit tests (default: ON)
- `CMAKE_BUILD_TYPE=Debug/Release` - Build configuration (default: Release)

### Environment Variables

- `BUILD_TYPE` - Build type (Debug/Release)
- `BUILD_DIR` - Build directory (default: build)
- `PARALLEL_JOBS` - Number of parallel build jobs

## Platform-Specific Notes

### Windows
- The client builds as `Client.dll` for injection into X3: Albion Prelude
- Visual Studio solution files are still available for IDE development
- MinGW is supported but Visual Studio is recommended

### Linux
- Builds as a static library for testing purposes
- All dependencies are available through package managers
- GCC and Clang are both supported

### macOS
- Requires Xcode Command Line Tools
- Use Homebrew for dependency management
- Apple Silicon (M1/M2) is supported

## Cross-Platform Features

The modernized X3MP includes several cross-platform improvements:

### New Reliability Protocol
- UDP packets now include sequence numbers and acknowledgments
- Automatic retransmission with configurable timeouts
- Duplicate packet filtering and out-of-order delivery

### Centralized Logging
- Runtime-configurable log levels
- Consistent formatting across C++ and Go components
- Thread-safe logging implementation

### Modern C++
- Replaced raw pointers with `std::unique_ptr` and `std::shared_ptr`
- RAII-based resource management
- Exception-safe code patterns

### Comprehensive Testing
- Unit tests for packet serialization
- Reliability protocol testing
- Logging system validation
- Cross-platform CI support

## Development

### Running Tests

```bash
# Go tests
cd GoServer && go test -v ./...

# C++ tests (after building)
cd build && ctest --output-on-failure
```

### Code Quality

```bash
# Go linting and formatting
cd GoServer && make check

# C++ formatting (if clang-format is available)
find Client -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

## Troubleshooting

### Common Issues

1. **CMake not found**: Install CMake 3.16+ from cmake.org
2. **Compiler errors**: Ensure C++17 support (GCC 7+, Clang 7+, MSVC 2019+)
3. **Missing dependencies**: Use package manager or vcpkg on Windows
4. **Go build fails**: Ensure Go 1.19+ is installed and `go mod tidy` runs successfully
5. **Tests fail**: Some network tests are skipped in CI environments

### Getting Help

- Check the build output for specific error messages
- Ensure all prerequisites are installed
- Try building individual components separately
- Use `--help` flag with build scripts for more options