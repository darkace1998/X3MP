# X3: Multiplayer (X3MP)

X3MP is an experimental, open-source multiplayer mod for the game **X3: Albion Prelude**.

## Project Status

**Warning:** This mod is in a very early stage of development. It is not stable, feature-complete, or suitable for a normal gameplay experience. Expect bugs, crashes, and missing features.

## Architecture

The mod uses a client-server architecture.
-   The **Client** is a DLL that is injected into the game to add multiplayer functionality.
-   The **Server** is a dedicated, standalone application that manages game state and synchronizes clients.

### The Go Server

The dedicated server is written in pure Go and is located in the `GoServer/` directory. It uses a standard UDP-based protocol for communication.

For instructions on how to build and run the server, see the [GoServer/README.md](GoServer/README.md) file.

### The C++ Client

The client is a C++ DLL that hooks into the X3 game engine. It is located in the `Client/` directory.

For instructions on how to build the client, see the [Client/README.md](Client/README.md) file.

## How to Use

For more information on the project's history, installation, and community discussion, please visit the official thread on the Egosoft forums:
[https://forum.egosoft.com/viewtopic.php?f=94&t=436927](https://forum.egosoft.com/viewtopic.php?f=94&t=436927)
