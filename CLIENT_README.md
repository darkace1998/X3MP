# X3MP Client - Timeout Fix Release

This directory contains the updated X3MP client with connection timeout improvements and build automation tools.

## 🚀 Quick Start (Windows Users)

### For Building the Client:
1. Ensure you have Visual Studio 2019/2022 with C++ tools installed
2. Download GameNetworkingSockets pre-built binaries 
3. Run: `build_client.bat`
4. Run: `create_release.bat` to package for distribution

### For Using Pre-built Client:
1. Download the latest release zip from GitHub releases
2. Extract to your X3 directory  
3. Follow INSTALL.txt instructions
4. Configure server settings in x3mp.xml

## 🔧 What's New

### Connection Stability Improvements
- ✅ **10-second connection timeout** - No more infinite waiting
- ✅ **Clear error messages** - User-friendly feedback when servers are down
- ✅ **Graceful failure handling** - Proper cleanup on connection issues  
- ✅ **Thread safety improvements** - Better resource management

### Before/After Behavior:
```
BEFORE: Client hangs indefinitely if server is unreachable
AFTER:  "Connection timeout! Unable to connect to server." (after 10s)

BEFORE: No feedback on connection status  
AFTER:  Clear in-game messages and console output
```

## 📁 File Structure

```
X3MP/
├── Client/                     # Client source code
│   ├── client.cpp             # ✨ Updated with timeout handling
│   ├── client.h               # ✨ New timeout constants & state 
│   ├── dllmain.cpp            # ✨ Improved connection waiting logic
│   └── [other files...]      # DirectX, hooking, UI components
├── X3Net/                     # Network protocol definitions
├── server-go/                 # Go server implementation  
├── build_client.bat           # ✨ NEW: Automated Windows build
├── create_release.bat         # ✨ NEW: Release packaging script
├── BUILD_GUIDE.md             # ✨ NEW: Comprehensive build guide
├── CROSS_COMPILE_REPORT.md    # ✨ NEW: Cross-compilation analysis
└── CLIENT_README.md           # ✨ NEW: This file
```

## 🛠 Build Tools

### `build_client.bat`
Automated build script for Windows:
- Detects Visual Studio/MSBuild installation
- Builds both Win32 DLL and x64 executable  
- Provides detailed build status and error reporting
- Checks for output files and shows sizes

### `create_release.bat` 
Release packaging automation:
- Creates structured release directory
- Copies binaries and dependencies
- Generates installation instructions
- Creates zip archive for distribution
- Includes changelog and documentation

### `BUILD_GUIDE.md`
Comprehensive manual build instructions:
- Step-by-step Visual Studio setup
- GameNetworkingSockets installation guide
- Troubleshooting common build errors
- Deployment and distribution notes

## 🔍 Technical Details

### Key Code Changes

**client.h** - New timeout infrastructure:
```cpp
const int CONNECTION_TIMEOUT_MS = 10000; // 10 second timeout

class Client {
    bool isConnected = false;
    bool hasConnectionFailed = false; 
    bool hasTimedOut = false;
    std::chrono::steady_clock::time_point connectionStartTime;
    bool CheckConnectionTimeout();
    // ...
};
```

**client.cpp** - Timeout checking in main loop:
```cpp
while (!g_bQuit) {
    PollIncomingMessages();
    PollConnectionStateChanges();
    
    if (!isConnected && !hasConnectionFailed && CheckConnectionTimeout()) {
        hasTimedOut = true;
        std::cout << "[ERR] Connection timeout after " << CONNECTION_TIMEOUT_MS/1000 << " seconds" << std::endl;
        Stop();
        break;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
```

**dllmain.cpp** - Enhanced waiting logic with user feedback:
```cpp
const int CONNECTION_WAIT_TIMEOUT_MS = 12000; // 12 seconds total wait time

while (!client.isConnected && !client.hasConnectionFailed && !client.hasTimedOut) {
    // Check for timeout
    if (elapsed.count() >= CONNECTION_WAIT_TIMEOUT_MS) {
        chatbox->SendChatMessage("Connection timeout! Unable to connect to server.", 255, 255, 100, 100);
        chatbox->SendChatMessage("Please check server address and try again.", 255, 255, 100, 100);
        // Clean exit
    }
    Sleep(100);
}
```

### Dependencies
- **GameNetworkingSockets**: Valve's networking library for reliable UDP
- **DirectX 9**: Graphics API for in-game overlay and rendering
- **Windows SDK**: For DLL injection and system API access  
- **pugixml**: XML parsing for configuration files

### Architecture
- **DLL Injection**: Client.dll injects into X3 process for game integration
- **DirectX Hooking**: Intercepts EndScene for overlay rendering
- **Memory Manipulation**: Hooks X3 functions for multiplayer data access
- **Network Layer**: GameNetworkingSockets handles client-server communication

## 🐛 Troubleshooting

### Build Issues
```
Error: GameNetworkingSockets headers not found
→ Download from GitHub releases and set include paths

Error: d3d9.lib not found  
→ Install Windows SDK with DirectX components

Error: MSBuild not found
→ Install Visual Studio with C++ development workload
```

### Runtime Issues
```
"Connection timeout" messages
→ Check server is running and accessible
→ Verify firewall/antivirus settings
→ Test with localhost (127.0.0.1) first

DLL injection fails
→ Run as administrator
→ Ensure X3 is fully loaded before injection
→ Check antivirus quarantine
```

## 🚦 Build Status

### Cross-Compilation (Linux → Windows) ❌
- **Status**: Not feasible due to Windows-specific dependencies
- **Reason**: GameNetworkingSockets, DirectX, DLL injection require Windows
- **Alternative**: Use Windows build environment or CI/CD

### Windows Native Build ✅
- **Status**: Fully supported with automated tools
- **Requirements**: Visual Studio 2019/2022, GameNetworkingSockets
- **Output**: Client.dll (32-bit), Client.exe (64-bit)

## 📦 Distribution

### Release Package Contents:
```
X3MP-Client-v1.5-timeout-fix.zip
├── bin/
│   ├── Client.dll              # Main client DLL
│   ├── Client.exe              # Standalone executable  
│   └── GameNetworkingSockets.dll # Required dependency
├── config/
│   └── x3mp.xml                # Default configuration
├── docs/
│   ├── BUILD_GUIDE.md          # Build instructions
│   └── CROSS_COMPILE_REPORT.md # Technical analysis
├── INSTALL.txt                 # Installation guide
└── CHANGELOG.txt               # Version history
```

### Installation Methods:
1. **DLL Injection** (recommended): Inject Client.dll into X3 process
2. **Standalone**: Run Client.exe alongside X3 (if supported)

## 🔮 Future Improvements

### Planned Enhancements:
- [ ] Configuration GUI for easier setup
- [ ] Auto-update mechanism for client distribution
- [ ] Better error reporting and diagnostic tools
- [ ] Plugin system for extensibility
- [ ] Cross-platform client rewrite (long-term)

### Architecture Considerations:
- Reduce Windows-specific dependencies for better portability
- Abstract DirectX components for OpenGL compatibility  
- Modularize networking layer for easier testing
- Improve build system with modern CMake integration

## 📞 Support

### Getting Help:
1. Check `BUILD_GUIDE.md` for detailed setup instructions
2. Review `CROSS_COMPILE_REPORT.md` for technical background  
3. Examine build output for specific error messages
4. Test with Go server (`server-go/`) for compatibility

### Common Solutions:
- **Build errors**: Verify all dependencies and paths are correct
- **Runtime crashes**: Check Visual C++ Redistributable is installed
- **Connection issues**: Test with local server first
- **Performance**: Use Release build, not Debug

---

*This client includes connection timeout improvements developed to make X3MP more stable and user-friendly. The enhanced error handling ensures players get immediate feedback when servers are unreachable instead of waiting indefinitely.*