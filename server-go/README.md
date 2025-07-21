# X3MP Go Server

A proof-of-concept implementation of the X3MP server in Go, demonstrating improved stability, concurrency, and cross-platform support.

## Features

- **Memory Safe**: Automatic garbage collection eliminates memory leaks
- **Concurrent**: Goroutines handle multiple clients efficiently  
- **Cross-Platform**: Single binary runs on Windows, Linux, macOS
- **Configurable**: JSON configuration with sensible defaults
- **Binary Compatible**: Maintains protocol compatibility with existing C++ client
- **Enhanced Logging**: Rich logging with emojis for player events and station management
- **Station Support**: Full support for space stations/stars with CreateStar packets
- **Graceful Shutdown**: Clean resource cleanup on exit

## Enhanced Features

### 🎮 Player Event Logging
- **Join Events**: `🎮 Player 1 (PlayerName) joined the server from IP:PORT with model X`
- **Leave Events**: `🚪 Player 1 (PlayerName) left the server (graceful disconnect/timed out)`
- **Chat Events**: `💬 Chat from PlayerName: message content`

### 🏭 Station Management System
- **Automatic Station Initialization**: Server creates default stations on startup
- **Station Broadcasting**: New players receive all existing stations
- **CreateStar Packet Support**: Full compatibility with C++ client station rendering
- **Default Stations**:
  - Central Station (ID: 2000) at position (0, 0, 0)
  - Trade Hub Alpha (ID: 2001) at position (100000, 0, 0) 
  - Mining Outpost (ID: 2002) at position (-100000, 50000, 0)
  - Research Station (ID: 2003) at position (0, -75000, 25000)

## Protocol Compatibility

This Go server implements the exact same network protocol as the original C++ server:

- `PacketConnect` - Player connection requests
- `PacketShipUpdate` - Ship position/rotation updates  
- `PacketChatMessage` - Chat messages between players
- `PacketCreateShip` - Ship creation notifications
- `PacketDeleteShip` - Ship removal notifications
- `PacketCreateStar` - Station/star creation notifications ⭐ **NEW**
- `PacketConnectAcknowledge` - Connection confirmation
- `PacketDisconnect` - Graceful player disconnection ⭐ **NEW**
- And more...

All packet structures match the original C++ layout using little-endian binary encoding.

## Building

```bash
go build -o x3mp-server main.go
```

## Running

```bash
./x3mp-server
```

The server will:
1. Load configuration from `server_config.json` (creates default if missing)
2. Start UDP listener on configured port (default: 13337)
3. Begin accepting client connections
4. Run game loop at configured tick rate (default: 60 FPS)

## Configuration

Edit `server_config.json`:

```json
{
  "server_port": 13337,
  "max_players": 32, 
  "server_name": "X3MP Go Server",
  "log_level": "info",
  "tick_rate": 60
}
```

## Advantages over C++ Version

### Stability
- **Memory Safety**: No buffer overflows, dangling pointers, or memory leaks
- **Error Handling**: Comprehensive error handling with proper propagation
- **Resource Management**: Automatic cleanup, no manual memory management

### Performance  
- **Concurrency**: Each client handled in separate goroutine
- **Non-blocking**: Asynchronous packet processing
- **Scalability**: Better performance under high player loads

### Development
- **Faster Iteration**: Quick compilation and deployment
- **Better Testing**: Built-in testing framework and benchmarking
- **Cross-Platform**: Single codebase for all platforms

### Operations
- **Single Binary**: No external dependencies 
- **Easy Deployment**: Copy single file, no DLL dependencies
- **Monitoring**: Built-in profiling and metrics collection
- **Containerization**: Easy Docker deployment

## Testing with Existing Client

The Go server is designed to work seamlessly with the existing C++ client. Simply:

1. Start the Go server: `./x3mp-server`
2. Launch the C++ client and connect to `localhost:13337`
3. All existing functionality should work identically

## Future Enhancements

With Go foundation in place, additional features become easier to implement:

- **Web Admin Interface**: Real-time server monitoring and control
- **Database Integration**: Persistent player data and statistics  
- **Load Balancing**: Multiple server instances with player distribution
- **Clustering**: Scale beyond single server limitations
- **Advanced Security**: Rate limiting, DDoS protection, encryption
- **Metrics & Analytics**: Detailed server performance monitoring
- **Plugin System**: Extensible server modifications
- **REST API**: External integration capabilities

## Migration Strategy

1. **Phase 1**: Run Go server in parallel for testing
2. **Phase 2**: Gradual migration of players to Go server  
3. **Phase 3**: Full replacement of C++ server
4. **Phase 4**: Add Go-exclusive advanced features

This allows risk-free evaluation while maintaining full backward compatibility.