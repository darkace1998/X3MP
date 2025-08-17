@echo off
setlocal enabledelayedexpansion

REM X3MP Cross-Platform Build Script for Windows
REM This script builds the X3MP project on Windows

REM Configuration
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release
if "%BUILD_DIR%"=="" set BUILD_DIR=build
if "%PARALLEL_JOBS%"=="" set PARALLEL_JOBS=4

REM Parse command line arguments
set BUILD_GO=true
set BUILD_CPP=true
set INSTALL_DEPS=false
set CLEAN_BUILD=false
set SHOW_HELP=false

:parse_args
if "%~1"=="" goto end_parse
if "%~1"=="-h" set SHOW_HELP=true
if "%~1"=="--help" set SHOW_HELP=true
if "%~1"=="-t" (
    set BUILD_TYPE=%~2
    shift
)
if "%~1"=="--type" (
    set BUILD_TYPE=%~2
    shift
)
if "%~1"=="-d" (
    set BUILD_DIR=%~2
    shift
)
if "%~1"=="--dir" (
    set BUILD_DIR=%~2
    shift
)
if "%~1"=="-j" (
    set PARALLEL_JOBS=%~2
    shift
)
if "%~1"=="--jobs" (
    set PARALLEL_JOBS=%~2
    shift
)
if "%~1"=="--go-only" (
    set BUILD_GO=true
    set BUILD_CPP=false
)
if "%~1"=="--cpp-only" (
    set BUILD_GO=false
    set BUILD_CPP=true
)
if "%~1"=="--install-deps" set INSTALL_DEPS=true
if "%~1"=="--clean" set CLEAN_BUILD=true
shift
goto parse_args
:end_parse

if "%SHOW_HELP%"=="true" goto show_help

echo ================================================
echo  X3MP Cross-Platform Build for Windows
echo ================================================
echo Build Type: %BUILD_TYPE%
echo Build Directory: %BUILD_DIR%
echo Parallel Jobs: %PARALLEL_JOBS%
echo.

REM Check for dependencies
where cmake >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake is not installed or not in PATH
    echo Please install CMake from https://cmake.org/
    exit /b 1
)

where go >nul 2>&1
if errorlevel 1 (
    if "%BUILD_GO%"=="true" (
        echo ERROR: Go is not installed or not in PATH
        echo Please install Go from https://golang.org/
        exit /b 1
    )
)

if "%INSTALL_DEPS%"=="true" (
    echo Installing dependencies...
    echo Please ensure you have Visual Studio with C++ workload installed
    echo For dependency management, consider using vcpkg:
    echo   git clone https://github.com/Microsoft/vcpkg.git
    echo   .\vcpkg\bootstrap-vcpkg.bat
    echo   .\vcpkg\vcpkg install gtest
)

if "%CLEAN_BUILD%"=="true" (
    echo Cleaning build directory...
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
)

REM Build Go Server
if "%BUILD_GO%"=="true" (
    echo.
    echo ================================================
    echo  Building Go Server
    echo ================================================
    
    cd GoServer
    if errorlevel 1 (
        echo ERROR: Cannot change to GoServer directory
        exit /b 1
    )
    
    echo Installing Go dependencies...
    go mod tidy
    if errorlevel 1 (
        echo ERROR: Failed to install Go dependencies
        exit /b 1
    )
    
    echo Running Go tests...
    go test -v ./...
    if errorlevel 1 (
        echo WARNING: Some Go tests failed
    )
    
    echo Building Go server...
    if not exist "bin" mkdir bin
    go build -o bin/x3mp_goserver.exe .
    if errorlevel 1 (
        echo ERROR: Failed to build Go server
        exit /b 1
    )
    
    cd ..
    echo SUCCESS: Go Server build completed
)

REM Build C++ Client
if "%BUILD_CPP%"=="true" (
    echo.
    echo ================================================
    echo  Building C++ Client
    echo ================================================
    
    if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
    cd "%BUILD_DIR%"
    
    echo Configuring CMake...
    cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DBUILD_TESTS=ON
    if errorlevel 1 (
        echo ERROR: CMake configuration failed
        exit /b 1
    )
    
    echo Building C++ client...
    cmake --build . --config %BUILD_TYPE% -j %PARALLEL_JOBS%
    if errorlevel 1 (
        echo ERROR: C++ build failed
        exit /b 1
    )
    
    echo Running C++ tests...
    ctest --output-on-failure -C %BUILD_TYPE%
    if errorlevel 1 (
        echo WARNING: Some C++ tests failed
    )
    
    cd ..
    echo SUCCESS: C++ Client build completed
)

echo.
echo ================================================
echo  Build Summary
echo ================================================

if "%BUILD_GO%"=="true" (
    if exist "GoServer\bin\x3mp_goserver.exe" (
        echo SUCCESS: Go Server: GoServer\bin\x3mp_goserver.exe
    ) else (
        echo ERROR: Go Server build failed
    )
)

if "%BUILD_CPP%"=="true" (
    if exist "%BUILD_DIR%\bin\Client.dll" (
        echo SUCCESS: C++ Client: %BUILD_DIR%\bin\Client.dll
    ) else (
        echo ERROR: C++ Client build failed
    )
)

echo.
echo Build completed!
goto end

:show_help
echo X3MP Cross-Platform Build Script for Windows
echo.
echo Usage: %~nx0 [OPTIONS]
echo.
echo Options:
echo   -h, --help          Show this help message
echo   -t, --type TYPE     Build type (Debug^|Release) [default: Release]
echo   -d, --dir DIR       Build directory [default: build]
echo   -j, --jobs N        Number of parallel jobs [default: 4]
echo   --go-only           Build only the Go server
echo   --cpp-only          Build only the C++ client
echo   --install-deps      Show dependency installation instructions
echo   --clean             Clean build directory before building
echo.
echo Environment Variables:
echo   BUILD_TYPE          Build type (same as --type)
echo   BUILD_DIR           Build directory (same as --dir)
echo   PARALLEL_JOBS       Number of parallel jobs (same as --jobs)
echo.
echo Examples:
echo   %~nx0                              # Build everything with default settings
echo   %~nx0 --type Debug --jobs 8        # Debug build with 8 parallel jobs
echo   %~nx0 --go-only                    # Build only Go server
echo   %~nx0 --install-deps               # Show dependency installation instructions

:end
endlocal