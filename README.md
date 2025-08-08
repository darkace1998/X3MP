# X3MP

This is an very very early stage multiplayer mod for the Game X3: Albion Prelude.
It isn't very playable nor stable.

## Server Architecture

The original C++ server in the `Server/` directory has been deprecated. A new, pure Go server is now located in the `GoServer/` directory. This new server uses a standard UDP networking protocol and does not rely on Steam Networking.

As part of this change, the C++ client in the `Client/` directory has been modified to use standard UDP sockets (via Winsock) instead of Steam Networking, allowing it to communicate with the new Go server.

For instructions on how to build and run the new server, please see the `GoServer/README.md` file.

# How to use

Please check out this thread on the Egosoft forum:
https://forum.egosoft.com/viewtopic.php?f=94&t=436927
