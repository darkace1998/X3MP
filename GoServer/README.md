# X3MP Go Server

This directory contains the new dedicated server for the X3MP mod, written in Go.

This server is a replacement for the original C++ server and uses standard UDP networking. It does **not** use Steam Networking Sockets.

## Building

To build the server, you need to have Go installed. From the root of the repository, run the following commands:

```sh
cd GoServer
go build
```

This will produce a server executable (`x3mp_goserver` on Linux/macOS or `x3mp_goserver.exe` on Windows).

## Running

Once built, you can run the server directly:

```sh
./x3mp_goserver
```

The server will start listening for UDP packets on port 13337.

## C++ Client Compatibility

The C++ client code in the `Client/` directory has been modified to be compatible with this server. The Steam Networking Sockets dependency has been removed and replaced with a standard UDP implementation using Winsock. You will need to recompile the client DLL for the changes to take effect.
