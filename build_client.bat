@echo off
echo ===============================================
echo         X3MP Client Build Script
echo ===============================================
echo.

REM Check if Visual Studio is installed
where msbuild >nul 2>&1
if errorlevel 1 (
    echo ERROR: MSBuild not found in PATH
    echo Please install Visual Studio 2019/2022 with C++ development tools
    echo Or run this script from Visual Studio Developer Command Prompt
    pause
    exit /b 1
)

REM Check if solution file exists
if not exist "X3MP.sln" (
    echo ERROR: X3MP.sln not found in current directory
    echo Please run this script from the X3MP repository root
    pause
    exit /b 1
)

echo Found MSBuild and solution file
echo.

REM Build Win32 Release (DLL for X3)
echo ===============================================
echo Building Win32 Release (Client.dll)...
echo ===============================================
msbuild X3MP.sln -p:Configuration=Release -p:Platform=Win32 -p:ProjectName=Client -m
if errorlevel 1 (
    echo.
    echo ERROR: Win32 build failed
    echo Check build output above for details
    pause
    exit /b 1
)

echo.
echo Win32 build completed successfully!
if exist "Client\Release\Client.dll" (
    echo - Output: Client\Release\Client.dll
) else (
    echo WARNING: Client.dll not found in expected location
)

echo.

REM Build x64 Release (Standalone executable)
echo ===============================================
echo Building x64 Release (Client.exe)...
echo ===============================================
msbuild X3MP.sln -p:Configuration=Release -p:Platform=x64 -p:ProjectName=Client -m
if errorlevel 1 (
    echo.
    echo ERROR: x64 build failed
    echo Check build output above for details
    pause
    exit /b 1
)

echo.
echo x64 build completed successfully!
if exist "x64\Release\Client.exe" (
    echo - Output: x64\Release\Client.exe
) else (
    echo WARNING: Client.exe not found in expected location
)

echo.
echo ===============================================
echo           Build Summary
echo ===============================================
echo Win32 DLL (for X3 game injection):
if exist "Client\Release\Client.dll" (
    echo   [OK] Client\Release\Client.dll
    for %%A in ("Client\Release\Client.dll") do echo   Size: %%~zA bytes
) else (
    echo   [FAIL] Client\Release\Client.dll - NOT FOUND
)

echo.
echo x64 Executable (standalone):  
if exist "x64\Release\Client.exe" (
    echo   [OK] x64\Release\Client.exe
    for %%A in ("x64\Release\Client.exe") do echo   Size: %%~zA bytes
) else (
    echo   [FAIL] x64\Release\Client.exe - NOT FOUND
)

echo.
echo ===============================================
echo Build completed! 
echo.
echo IMPORTANT: For distribution, you also need:
echo - GameNetworkingSockets.dll (runtime dependency)
echo - Visual C++ Redistributable 2019/2022
echo.
echo Refer to BUILD_GUIDE.md for setup instructions
echo ===============================================
pause