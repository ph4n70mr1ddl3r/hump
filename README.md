# Heads-up NLHE Bot Server

A C++ server hosting a single heads-up No Limit Texas Hold'em table for two WebSocket clients.

## Features

- Standard NLHE rules with configurable timeouts
- Random-strategy bots with human-like delays
- Automatic stack top-up when low
- Graceful disconnection handling with reconnection support
- WebSocket API with JSON message format

## Quick Start

See [specs/001-heads-up-nlhe-bots/quickstart.md](specs/001-heads-up-nlhe-bots/quickstart.md) for detailed instructions.

### Prerequisites

- Linux environment (Ubuntu 20.04+ recommended)
- C++17 compiler (g++ 9+ or clang 10+)
- CMake 3.15+
- Boost libraries (Boost.Beast, Boost.Asio, Boost.System)
- nlohmann/json (header-only)
- Google Test (optional, for running tests)

### Building

```bash
git clone <repository-url>
cd <repository>
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

This produces two executables:

- `server/poker_server`
- `client/poker_bot`

### Running the Server

```bash
./server/poker_server --port 8080 --ample-time 30 --removal-timeout 60
```

### Running the Client (Bot)

```bash
./client/poker_bot --server ws://localhost:8080 --name RandomBot
```

Run two instances in separate terminals to start a heads‑up game.

## Testing

To run the unit tests:

```bash
cd build
ctest --output-on-failure
```

Integration tests (require server running) are available in `tests/integration/`.

## Documentation

- [Specification](specs/001-heads-up-nlhe-bots/spec.md)
- [Implementation Plan](specs/001-heads-up-nlhe-bots/plan.md)
- [Data Model](specs/001-heads-up-nlhe-bots/data-model.md)
- [WebSocket API Contract](specs/001-heads-up-nlhe-bots/contracts/websocket-api.md)
- [Research & Technology Decisions](specs/001-heads-up-nlhe-bots/research.md)

## License

[Add license information]
