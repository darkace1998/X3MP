# X3MP Client

This directory contains the source code for the X3MP client. The client is a DLL file that is injected into the `X3: Albion Prelude` game process. It handles communication with the game server and modifies the game in memory to enable multiplayer functionality.

## Building the Client

The client is a C++ project designed to be built with Visual Studio.

1.  **Open the Solution:** Open the `X3MP.sln` file located in the root of the repository with Visual Studio.
2.  **Select the Build Configuration:** Choose the appropriate build configuration (e.g., "Debug" or "Release").
3.  **Build the Project:** Build the "Client" project from within Visual Studio. This will produce a `Client.dll` file in the build output directory.

## Dependencies

-   **X3: Albion Prelude:** You must have a copy of the game.
-   **Visual Studio:** A version that can open the included `.sln` file (e.g., Visual Studio 2019 or newer) with the "Desktop development with C++" workload installed.

## Usage

The compiled `Client.dll` is loaded by the `X3MP_Launcher`. The launcher is responsible for injecting the DLL into the game process.
