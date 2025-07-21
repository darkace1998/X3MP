# X3MP Client Build Guide

This document explains how to build the updated X3MP client with connection timeout improvements on Windows.

## Prerequisites

### Required Software
1. **Visual Studio 2019 or 2022** (Community Edition is free)
   - Install the "Desktop development with C++" workload
   - Ensure Windows 10/11 SDK is installed

2. **Git** for Windows
   - Download from: https://git-scm.com/download/win

### Required Libraries

#### 1. GameNetworkingSockets
The client uses Valve's GameNetworkingSockets library for networking.

**Option A: Pre-built binaries (Recommended)**
1. Download pre-built GameNetworkingSockets from the official releases:
   - https://github.com/ValveSoftware/GameNetworkingSockets/releases
2. Extract to `C:\GameNetworkingSockets\`
3. Ensure the following structure exists:
   ```
   C:\GameNetworkingSockets\
   ├── include\
   │   └── GameNetworkingSockets\
   │       └── steam\
   │           ├── steamnetworkingsockets.h
   │           ├── isteamnetworkingutils.h
   │           └── steam_api.h
   └── lib\
       ├── Win32\
       │   └── GameNetworkingSockets.lib
       └── x64\
           └── GameNetworkingSockets.lib
   ```

**Option B: Build from source**
```batch
git clone https://github.com/ValveSoftware/GameNetworkingSockets.git
cd GameNetworkingSockets
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A x64 ..
cmake --build . --config Release
```

#### 2. DirectX SDK (Legacy)
The client uses DirectX 9 libraries. These are typically included with Visual Studio, but if missing:

1. Download Microsoft DirectX SDK (June 2010)
2. Install to default location: `C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\`

## Build Instructions

### Step 1: Clone the Repository
```batch
git clone https://github.com/darkace1998/X3MP.git
cd X3MP
```

### Step 2: Update Project Settings
1. Open `X3MP.sln` in Visual Studio
2. Right-click the **Client** project → Properties
3. Under **C/C++** → **General** → **Additional Include Directories**, add:
   - `C:\GameNetworkingSockets\include`
   - `C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include` (if using DirectX SDK)

4. Under **Linker** → **General** → **Additional Library Directories**, add:
   - `C:\GameNetworkingSockets\lib\x64` (for x64 build)
   - `C:\GameNetworkingSockets\lib\Win32` (for Win32 build)
   - `C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x64` (if using DirectX SDK)

5. Under **Linker** → **Input** → **Additional Dependencies**, add:
   - `GameNetworkingSockets.lib`
   - `d3d9.lib`
   - `d3dx9.lib`

### Step 3: Build the Client

#### For Win32 (32-bit) DLL:
1. Set solution configuration to **Release** and platform to **Win32**
2. Build → Build Solution (or press Ctrl+Shift+B)
3. The compiled DLL will be in: `Client\Release\Client.dll`

#### For x64 (64-bit) Application:
1. Set solution configuration to **Release** and platform to **x64**
2. Build → Build Solution (or press Ctrl+Shift+B)
3. The compiled executable will be in: `x64\Release\Client.exe`

## Recent Improvements

The updated client includes:
- ✅ **Connection Timeout Handling**: 10-second network timeout
- ✅ **User-Friendly Error Messages**: Clear feedback when server is unreachable
- ✅ **Graceful Exit**: Proper cleanup on connection failures
- ✅ **Thread Safety**: Improved connection state management

### Key Features Added:
```cpp
// Connection timeout constants
const int CONNECTION_TIMEOUT_MS = 10000; // 10 second timeout

// New client state management
bool isConnected = false;
bool hasConnectionFailed = false; 
bool hasTimedOut = false;
std::chrono::steady_clock::time_point connectionStartTime;
```

## Troubleshooting

### Common Build Errors

**Error: Cannot find GameNetworkingSockets headers**
- Solution: Verify GameNetworkingSockets is installed correctly and paths are added to project

**Error: Cannot find d3d9.lib or d3dx9.lib**
- Solution: Install DirectX SDK or ensure Windows SDK includes DirectX libraries

**Error: LNK2001 unresolved external symbol**
- Solution: Check that all required .lib files are added to linker dependencies

**Error: Cannot open include file 'steam_api.h'**
- Solution: Either install Steam SDK or comment out the Steam-specific includes if not needed

### Testing the Build

1. Copy the compiled DLL/EXE to your X3 game directory
2. Ensure GameNetworkingSockets.dll is in the same directory or system PATH
3. Test connection timeout by attempting to connect to a non-existent server
4. Verify you receive timeout messages after 10 seconds

## Deployment

### For Distribution:
1. Copy the compiled client (Client.dll or Client.exe)
2. Include GameNetworkingSockets.dll runtime
3. Include any Visual C++ Redistributable requirements
4. Create a zip package with all dependencies

### Runtime Requirements:
- Visual C++ Redistributable 2019/2022 (x64 or x86)
- GameNetworkingSockets.dll
- Windows 10/11 (DirectX 9 compatible)

## Support

If you encounter build issues:
1. Check that all prerequisites are installed
2. Verify project configuration matches your target platform
3. Ensure all library paths are correct
4. Check Visual Studio Output window for detailed error messages

For X3:Albion Prelude compatibility, ensure:
- The game is patched to the latest version
- No conflicting mods are installed
- The client DLL architecture matches your game version (32-bit vs 64-bit)