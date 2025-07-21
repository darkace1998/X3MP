@echo off
echo ===============================================
echo      X3MP Client Release Package Creator
echo ===============================================
echo.

REM Set version and output directory
set VERSION=1.5-timeout-fix
set OUTPUT_DIR=X3MP-Client-%VERSION%
set ZIP_NAME=X3MP-Client-%VERSION%.zip

REM Clean up previous builds
if exist "%OUTPUT_DIR%" rmdir /s /q "%OUTPUT_DIR%"
if exist "%ZIP_NAME%" del "%ZIP_NAME%"

REM Create release directory structure
echo Creating release directory structure...
mkdir "%OUTPUT_DIR%"
mkdir "%OUTPUT_DIR%\bin"
mkdir "%OUTPUT_DIR%\config"
mkdir "%OUTPUT_DIR%\docs"
mkdir "%OUTPUT_DIR%\redist"

REM Check if builds exist
if not exist "Client\Release\Client.dll" (
    echo ERROR: Client.dll not found. Please run build_client.bat first.
    pause
    exit /b 1
)

REM Copy main binaries
echo Copying client binaries...
copy "Client\Release\Client.dll" "%OUTPUT_DIR%\bin\" >nul
if exist "x64\Release\Client.exe" copy "x64\Release\Client.exe" "%OUTPUT_DIR%\bin\" >nul

REM Copy dependencies (user needs to provide these)
echo Copying dependencies...
if exist "GameNetworkingSockets.dll" (
    copy "GameNetworkingSockets.dll" "%OUTPUT_DIR%\bin\" >nul
) else (
    echo WARNING: GameNetworkingSockets.dll not found in root directory
    echo Please copy it manually to the release package
)

REM Create default configuration
echo Creating default configuration...
echo ^<?xml version="1.0" encoding="utf-8"?^> > "%OUTPUT_DIR%\config\x3mp.xml"
echo ^<settings^> >> "%OUTPUT_DIR%\config\x3mp.xml"
echo   ^<username^>Player^</username^> >> "%OUTPUT_DIR%\config\x3mp.xml"  
echo   ^<ip^>127.0.0.1^</ip^> >> "%OUTPUT_DIR%\config\x3mp.xml"
echo   ^<port^>13337^</port^> >> "%OUTPUT_DIR%\config\x3mp.xml"
echo ^</settings^> >> "%OUTPUT_DIR%\config\x3mp.xml"

REM Copy documentation
echo Copying documentation...
copy "BUILD_GUIDE.md" "%OUTPUT_DIR%\docs\" >nul
copy "CROSS_COMPILE_REPORT.md" "%OUTPUT_DIR%\docs\" >nul
if exist "README.md" copy "README.md" "%OUTPUT_DIR%\docs\" >nul

REM Create installation instructions
echo Creating installation instructions...
echo X3MP Client Installation Instructions > "%OUTPUT_DIR%\INSTALL.txt"
echo ======================================= >> "%OUTPUT_DIR%\INSTALL.txt"
echo. >> "%OUTPUT_DIR%\INSTALL.txt"
echo 1. PREREQUISITES: >> "%OUTPUT_DIR%\INSTALL.txt"
echo    - X3: Albion Prelude (latest version) >> "%OUTPUT_DIR%\INSTALL.txt"
echo    - Visual C++ Redistributable 2019/2022 (x64) >> "%OUTPUT_DIR%\INSTALL.txt"
echo      Download from: https://aka.ms/vs/17/release/vc_redist.x64.exe >> "%OUTPUT_DIR%\INSTALL.txt"
echo. >> "%OUTPUT_DIR%\INSTALL.txt"
echo 2. CLIENT INSTALLATION: >> "%OUTPUT_DIR%\INSTALL.txt"
echo    a) For DLL Injection (recommended): >> "%OUTPUT_DIR%\INSTALL.txt"
echo       - Copy bin\Client.dll to your X3 game directory >> "%OUTPUT_DIR%\INSTALL.txt"
echo       - Copy bin\GameNetworkingSockets.dll to X3 game directory >> "%OUTPUT_DIR%\INSTALL.txt"
echo       - Copy config\x3mp.xml to X3 game directory >> "%OUTPUT_DIR%\INSTALL.txt"
echo       - Use X3MP Launcher or DLL injector to load Client.dll >> "%OUTPUT_DIR%\INSTALL.txt"
echo. >> "%OUTPUT_DIR%\INSTALL.txt"
echo    b) For Standalone (if available): >> "%OUTPUT_DIR%\INSTALL.txt"
echo       - Copy bin\Client.exe to desired location >> "%OUTPUT_DIR%\INSTALL.txt"
echo       - Copy bin\GameNetworkingSockets.dll to same directory as Client.exe >> "%OUTPUT_DIR%\INSTALL.txt"
echo       - Copy config\x3mp.xml to same directory as Client.exe >> "%OUTPUT_DIR%\INSTALL.txt"
echo       - Run Client.exe while X3 is running >> "%OUTPUT_DIR%\INSTALL.txt"
echo. >> "%OUTPUT_DIR%\INSTALL.txt"
echo 3. CONFIGURATION: >> "%OUTPUT_DIR%\INSTALL.txt"
echo    - Edit x3mp.xml to set your username and server details >> "%OUTPUT_DIR%\INSTALL.txt"
echo    - Default server: 127.0.0.1:13337 (localhost) >> "%OUTPUT_DIR%\INSTALL.txt"
echo. >> "%OUTPUT_DIR%\INSTALL.txt"
echo 4. NEW FEATURES IN THIS VERSION: >> "%OUTPUT_DIR%\INSTALL.txt"
echo    - Connection timeout handling (10 second timeout) >> "%OUTPUT_DIR%\INSTALL.txt"
echo    - Better error messages when server is unreachable >> "%OUTPUT_DIR%\INSTALL.txt"
echo    - Graceful exit on connection failures >> "%OUTPUT_DIR%\INSTALL.txt"
echo    - Improved thread safety and cleanup >> "%OUTPUT_DIR%\INSTALL.txt"
echo. >> "%OUTPUT_DIR%\INSTALL.txt"
echo 5. TROUBLESHOOTING: >> "%OUTPUT_DIR%\INSTALL.txt"
echo    - If you get "Connection timeout" messages: >> "%OUTPUT_DIR%\INSTALL.txt"
echo      * Check server is running on specified IP:port >> "%OUTPUT_DIR%\INSTALL.txt"
echo      * Verify firewall/antivirus not blocking connections >> "%OUTPUT_DIR%\INSTALL.txt"
echo      * Try connecting to localhost (127.0.0.1) first >> "%OUTPUT_DIR%\INSTALL.txt"
echo    - If DLL injection fails: >> "%OUTPUT_DIR%\INSTALL.txt"
echo      * Ensure X3 is running and fully loaded >> "%OUTPUT_DIR%\INSTALL.txt"
echo      * Run injector as administrator >> "%OUTPUT_DIR%\INSTALL.txt"
echo      * Check antivirus isn't quarantining the DLL >> "%OUTPUT_DIR%\INSTALL.txt"
echo. >> "%OUTPUT_DIR%\INSTALL.txt"
echo For more details, see docs\BUILD_GUIDE.md >> "%OUTPUT_DIR%\INSTALL.txt"

REM Create changelog
echo Creating changelog...
echo X3MP Client Changelog > "%OUTPUT_DIR%\CHANGELOG.txt"
echo ===================== >> "%OUTPUT_DIR%\CHANGELOG.txt"
echo. >> "%OUTPUT_DIR%\CHANGELOG.txt"
echo Version %VERSION% >> "%OUTPUT_DIR%\CHANGELOG.txt"
echo - Added connection timeout handling (10 second timeout) >> "%OUTPUT_DIR%\CHANGELOG.txt"
echo - Improved error messages for connection failures >> "%OUTPUT_DIR%\CHANGELOG.txt"  
echo - Added graceful exit when server is unreachable >> "%OUTPUT_DIR%\CHANGELOG.txt"
echo - Enhanced thread safety and cleanup procedures >> "%OUTPUT_DIR%\CHANGELOG.txt"
echo - Better user feedback with in-game chat messages >> "%OUTPUT_DIR%\CHANGELOG.txt"
echo - Fixed infinite waiting when server is down >> "%OUTPUT_DIR%\CHANGELOG.txt"
echo. >> "%OUTPUT_DIR%\CHANGELOG.txt"
echo Previous versions: >> "%OUTPUT_DIR%\CHANGELOG.txt"
echo - Initial multiplayer client implementation >> "%OUTPUT_DIR%\CHANGELOG.txt"
echo - DirectX 9 integration for in-game overlay >> "%OUTPUT_DIR%\CHANGELOG.txt"
echo - GameNetworkingSockets networking layer >> "%OUTPUT_DIR%\CHANGELOG.txt"
echo - DLL injection for X3 game integration >> "%OUTPUT_DIR%\CHANGELOG.txt"

REM Check for 7zip or other compression tools
where 7z >nul 2>&1
if not errorlevel 1 (
    echo Creating zip archive with 7-Zip...
    7z a -tzip "%ZIP_NAME%" "%OUTPUT_DIR%\*" >nul
    goto :zip_done
)

where powershell >nul 2>&1
if not errorlevel 1 (
    echo Creating zip archive with PowerShell...
    powershell -command "Compress-Archive -Path '%OUTPUT_DIR%\*' -DestinationPath '%ZIP_NAME%'"
    goto :zip_done
)

echo WARNING: No compression tool found (7-Zip or PowerShell)
echo Please manually zip the "%OUTPUT_DIR%" folder
goto :skip_zip

:zip_done
echo Archive created: %ZIP_NAME%

:skip_zip

REM Display summary
echo.
echo ===============================================
echo         Release Package Summary
echo ===============================================
echo Output Directory: %OUTPUT_DIR%
if exist "%ZIP_NAME%" (
    echo Zip Archive: %ZIP_NAME%
    for %%A in ("%ZIP_NAME%") do echo Archive Size: %%~zA bytes
)

echo.
echo Contents:
echo   bin\
if exist "%OUTPUT_DIR%\bin\Client.dll" (
    echo     Client.dll (Win32 DLL for X3 injection^) ✓
) else (
    echo     Client.dll (Win32 DLL for X3 injection^) ✗
)

if exist "%OUTPUT_DIR%\bin\Client.exe" (
    echo     Client.exe (x64 standalone^) ✓
) else (
    echo     Client.exe (x64 standalone^) ✗ 
)

if exist "%OUTPUT_DIR%\bin\GameNetworkingSockets.dll" (
    echo     GameNetworkingSockets.dll ✓
) else (
    echo     GameNetworkingSockets.dll ✗ ^(manual copy required^)
)

echo   config\
echo     x3mp.xml (default configuration^) ✓
echo   docs\
echo     BUILD_GUIDE.md ✓
echo     CROSS_COMPILE_REPORT.md ✓
echo   INSTALL.txt ✓
echo   CHANGELOG.txt ✓

echo.
echo ===============================================
if exist "%ZIP_NAME%" (
    echo Ready for distribution: %ZIP_NAME%
) else (
    echo Ready for manual packaging: %OUTPUT_DIR%
)
echo ===============================================
pause