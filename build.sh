#!/bin/bash

# X3MP Cross-Platform Build Script
# This script builds the X3MP project on various platforms

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_TYPE=${BUILD_TYPE:-Release}
BUILD_DIR=${BUILD_DIR:-build}
PARALLEL_JOBS=${PARALLEL_JOBS:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}

# Functions
print_header() {
    echo -e "${BLUE}================================================${NC}"
    echo -e "${BLUE} $1${NC}"
    echo -e "${BLUE}================================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

detect_platform() {
    case "$(uname -s)" in
        Linux*)     PLATFORM=Linux;;
        Darwin*)    PLATFORM=Mac;;
        CYGWIN*|MINGW*|MSYS*) PLATFORM=Windows;;
        *)          PLATFORM="UNKNOWN";;
    esac
    echo "Detected platform: $PLATFORM"
}

install_dependencies() {
    print_header "Installing Dependencies"
    
    case $PLATFORM in
        Linux)
            if command -v apt-get &> /dev/null; then
                sudo apt-get update
                sudo apt-get install -y cmake build-essential libgtest-dev
            elif command -v yum &> /dev/null; then
                sudo yum install -y cmake gcc-c++ gtest-devel
            elif command -v pacman &> /dev/null; then
                sudo pacman -S --noconfirm cmake gcc gtest
            else
                print_warning "Unknown Linux distribution. Please install cmake, gcc, and gtest manually."
            fi
            ;;
        Mac)
            if command -v brew &> /dev/null; then
                brew install cmake googletest
            else
                print_warning "Homebrew not found. Please install cmake and googletest manually."
            fi
            ;;
        Windows)
            print_warning "Windows detected. Please ensure you have Visual Studio or MinGW installed."
            print_warning "CMake and vcpkg are recommended for dependency management."
            ;;
        *)
            print_error "Unsupported platform: $PLATFORM"
            exit 1
            ;;
    esac
}

build_goserver() {
    print_header "Building Go Server"
    
    if ! command -v go &> /dev/null; then
        print_error "Go is not installed. Please install Go 1.19 or later."
        return 1
    fi
    
    cd GoServer
    
    # Install dependencies
    go mod tidy
    
    # Run tests
    echo "Running Go tests..."
    go test -v ./...
    
    # Build the server
    echo "Building Go server..."
    go build -o bin/x3mp_goserver .
    
    cd ..
    print_success "Go Server build completed"
}

build_client() {
    print_header "Building C++ Client"
    
    if ! command -v cmake &> /dev/null; then
        print_error "CMake is not installed."
        return 1
    fi
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Configure
    echo "Configuring CMake..."
    case $PLATFORM in
        Windows)
            # Try to use Visual Studio generator if available
            if command -v "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" &> /dev/null; then
                cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DBUILD_TESTS=ON
            else
                cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DBUILD_TESTS=ON
            fi
            ;;
        *)
            cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DBUILD_TESTS=ON
            ;;
    esac
    
    # Build
    echo "Building C++ client..."
    cmake --build . --config "$BUILD_TYPE" -j "$PARALLEL_JOBS"
    
    # Run tests if available
    if [ "$BUILD_TYPE" != "Release" ] || [ -f "tests/x3mp_tests" ] || [ -f "tests/x3mp_tests.exe" ]; then
        echo "Running C++ tests..."
        ctest --output-on-failure -C "$BUILD_TYPE" || print_warning "Some tests failed"
    fi
    
    cd ..
    print_success "C++ Client build completed"
}

show_help() {
    echo "X3MP Cross-Platform Build Script"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -t, --type TYPE     Build type (Debug|Release) [default: Release]"
    echo "  -d, --dir DIR       Build directory [default: build]"
    echo "  -j, --jobs N        Number of parallel jobs [default: auto-detect]"
    echo "  --go-only           Build only the Go server"
    echo "  --cpp-only          Build only the C++ client"
    echo "  --install-deps      Install system dependencies"
    echo "  --clean             Clean build directory before building"
    echo ""
    echo "Environment Variables:"
    echo "  BUILD_TYPE          Build type (same as --type)"
    echo "  BUILD_DIR           Build directory (same as --dir)"
    echo "  PARALLEL_JOBS       Number of parallel jobs (same as --jobs)"
    echo ""
    echo "Examples:"
    echo "  $0                              # Build everything with default settings"
    echo "  $0 --type Debug --jobs 8        # Debug build with 8 parallel jobs"
    echo "  $0 --go-only                    # Build only Go server"
    echo "  $0 --install-deps               # Install dependencies only"
}

# Parse command line arguments
BUILD_GO=true
BUILD_CPP=true
INSTALL_DEPS=false
CLEAN_BUILD=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -d|--dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -j|--jobs)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        --go-only)
            BUILD_GO=true
            BUILD_CPP=false
            shift
            ;;
        --cpp-only)
            BUILD_GO=false
            BUILD_CPP=true
            shift
            ;;
        --install-deps)
            INSTALL_DEPS=true
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use --help for usage information."
            exit 1
            ;;
    esac
done

# Main execution
print_header "X3MP Cross-Platform Build"
echo "Build Type: $BUILD_TYPE"
echo "Build Directory: $BUILD_DIR"
echo "Parallel Jobs: $PARALLEL_JOBS"
echo ""

detect_platform

if [ "$INSTALL_DEPS" = true ]; then
    install_dependencies
fi

if [ "$CLEAN_BUILD" = true ]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# Store original directory
ORIGINAL_DIR=$(pwd)

if [ "$BUILD_GO" = true ]; then
    build_goserver
    cd "$ORIGINAL_DIR"
fi

if [ "$BUILD_CPP" = true ]; then
    build_client
    cd "$ORIGINAL_DIR"
fi

print_header "Build Summary"
if [ "$BUILD_GO" = true ]; then
    if [ -f "GoServer/bin/x3mp_goserver" ]; then
        print_success "Go Server: GoServer/bin/x3mp_goserver"
    else
        print_error "Go Server build failed"
    fi
fi

if [ "$BUILD_CPP" = true ]; then
    if [ -f "$BUILD_DIR/bin/Client.dll" ] || [ -f "$BUILD_DIR/bin/libClient.a" ]; then
        print_success "C++ Client: $BUILD_DIR/bin/"
    else
        print_error "C++ Client build failed"
    fi
fi

print_success "Build completed!"