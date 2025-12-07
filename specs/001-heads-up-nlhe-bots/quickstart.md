# Quick Start

## Prerequisites

- Linux environment (Ubuntu 20.04+ recommended)
- C++17 compiler (g++ 9+ or clang 10+)
- CMake 3.15+
- Boost libraries (Boost.Beast, Boost.Asio, Boost.System)
- nlohmann/json (header‑only, can be fetched by CMake)
- Google Test (optional, for running tests)

## Installation

### Ubuntu/Debian

```bash
sudo apt update
sudo apt install -y g++ cmake libboost-all-dev nlohmann-json3-dev googletest
```

### Other Distributions

Install equivalent packages for Boost, CMake, and a C++17 compiler.

## Building the Project

```bash
git clone <repository-url>
cd <repository>
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

This will produce two executables:
- `server/poker_server`
- `client/poker_bot`

## Running the Server

```bash
./server/poker_server --port 8080 --ample-time 30 --removal-timeout 60
```

Options:
- `--port`: WebSocket port (default 8080)
- `--ample-time`: grace period for disconnections in seconds (default 30)
- `--removal-timeout`: time after which disconnected players are removed (default 60)

The server will wait for two clients to connect.

## Running the Client (Bot)

```bash
./client/poker_bot --server ws://localhost:8080 --name RandomBot
```

Options:
- `--server`: WebSocket URL of the server
- `--name`: display name (optional)

Run two instances (in separate terminals) to start a heads‑up game.

## Testing

To run the unit tests:

```bash
cd build
ctest --output-on-failure
```

Integration tests (require server running) are available in `tests/integration/`.

## Configuration

Server configuration is provided exclusively via command‑line arguments (see “Running the Server” above). No configuration files are used, per spec requirement.

### Bot Configuration

### Bot Configuration

Bot behavior can be tuned via environment variables:

- `BOT_MIN_DELAY_MS`: minimum thinking delay (default 500)
- `BOT_MAX_DELAY_MS`: maximum thinking delay (default 3000)
- `BOT_TOPUP_THRESHOLD_BB`: stack threshold for auto‑top‑up (default 5)

## Development

### Code Style

Format code with clang‑format:

```bash
find src -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i
```

Run clang‑tidy for static analysis:

```bash
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
run-clang-tidy
```

### Adding New Features

1. Follow TDD: write tests first in `tests/unit/`.
2. Implement in `src/core/` for shared logic, `src/server/` for server‑side, `src/client/` for bot logic.
3. Update the WebSocket message contracts in `specs/001-heads-up-nlhe-bots/contracts/`.
4. Run integration tests to ensure compatibility.

## Troubleshooting

- **Connection refused**: Ensure server is running and listening on the correct port.
- **Boost not found**: Install Boost development packages (`libboost-all-dev`).
- **JSON library missing**: CMake will automatically download nlohmann/json; otherwise install `nlohmann-json3-dev`.
- **Random decisions seem predictable**: The bot uses `std::mt19937` seeded from `std::random_device`; ensure your system provides entropy.

## Next Steps

- Review the full specification in `specs/001-heads-up-nlhe-bots/spec.md`.
- Examine the implementation plan in `specs/001-heads-up-nlhe-bots/plan.md`.
- Explore the data model and API contracts in the same directory.