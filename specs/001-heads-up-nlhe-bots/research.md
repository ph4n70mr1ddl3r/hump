# Research & Technology Decisions

## C++ Version

**Decision**: C++17

**Rationale**: C++17 provides modern features (structured bindings, std::optional, std::variant, filesystem) while maintaining broad compiler support. It strikes a balance between modernity and stability. C++20 offers coroutines and networking TS but adoption is still growing; C++17 is sufficient for this project.

**Alternatives considered**:
- C++11: Too old, missing convenient modern features.
- C++20: Not yet universally supported; coroutines not required for this scale.

## WebSocket Library

**Decision**: Boost.Beast

**Rationale**: Part of Boost libraries, well-documented, supports asynchronous operations, integrates with Boost.Asio for networking. Provides both HTTP and WebSocket capabilities, which could be useful for future extensions (e.g., status endpoints). Heavyweight but acceptable for a single‑table server.

**Alternatives considered**:
- uWebSockets: Lightweight and fast, but less feature‑rich and less integrated with C++ standard patterns.
- WebSocket++: Another lightweight option, but Boost.Beast is more widely used in production C++ projects.

## JSON Library

**Decision**: nlohmann/json

**Rationale**: Header‑only, easy integration, intuitive API, excellent documentation, and widely adopted in the C++ community. Supports modern C++ features and provides convenient serialization/deserialization.

**Alternatives considered**:
- RapidJSON: Faster but more complex API.
- Boost.PropertyTree: Limited JSON support, not ideal.

## Testing Framework

**Decision**: Google Test (gtest)

**Rationale**: Industry standard, integrates well with CMake, provides rich assertion macros, mock support (Google Mock), and extensive tooling (test discovery, XML output). Widely used in C++ projects.

**Alternatives considered**:
- Catch2: Also popular, but Google Test has more mature integration with CI/CD systems.
- Boost.Test: Heavyweight and less intuitive.

## Target Platform

**Decision**: Linux (server and client)

**Rationale**: The project will be developed and deployed on Linux environments. No need for cross‑platform support at this stage. If required later, the code can be made portable (avoiding platform‑specific calls).

**Alternatives considered**:
- Cross‑platform (Linux/macOS/Windows): Adds complexity; not required by the specification.

## Project Structure

**Decision**: Single CMake project with multiple executables and a shared core library.

**Rationale**: Keeps the codebase cohesive; server and client can share poker logic (hand ranking, betting rules) while having separate entry points. CMake is the de‑facto standard for C++ and works well with the chosen dependencies.

**Alternatives considered**:
- Separate repositories for server and client: Overkill for a small project.
- Bazel / Meson: Less common in the C++ ecosystem compared to CMake.

## Networking & Concurrency Model

**Decision**: Asynchronous I/O using Boost.Asio (via Boost.Beast) with a single‑threaded event loop.

**Rationale**: A single‑threaded event loop is sufficient for a two‑player table; it simplifies state management and avoids locking. Asynchronous I/O allows handling multiple clients without threads, and integrates naturally with timers for disconnection timeouts.

**Alternatives considered**:
- Multi‑threaded per‑connection: Unnecessary complexity, risk of race conditions.
- Blocking I/O with threads: Would require synchronisation and add overhead.

## Poker Logic Implementation

**Decision**: Implement hand ranking and betting logic in a standalone, unit‑tested C++ library.

**Rationale**: Poker rules are deterministic and independent of networking; a separate library ensures correctness and testability. Use existing open‑source poker hand evaluators if appropriate (e.g., “Cactus Kev’s” algorithm for performance), but a straightforward implementation is acceptable for two players.

**Alternatives considered**:
- Integrating a third‑party poker engine: Adds dependency; not needed for basic NLHE.
- Implementing logic directly in server code: Harder to test and maintain.

## Handling Disconnections & Timeouts

**Decision**: Server maintains a timer per client; after a configurable “ample time” (default 30 seconds) the client is marked folded. After a longer “removal timeout” (default 1 minute) the client is removed from the table.

**Rationale**: Timers are naturally expressed with Boost.Asio’s `deadline_timer`. The state machine must track disconnected players and allow re‑connection within the grace period.

**Alternatives considered**:
- Immediate folding on disconnect: Too harsh, does not match requirement of “ample time”.
- Complex session recovery with persistence: Overkill for a non‑persistent server.

## Random Number Generation

**Decision**: Use `<random>` facilities (std::mt19937) for bot decisions and delays.

**Rationale**: Standard C++ library provides high‑quality pseudo‑random generators; no need for cryptographic security. Seed from `std::random_device`.

**Alternatives considered**:
- External library (e.g., PCG): Unnecessary complexity.
- C `rand()`: Low quality and not thread‑safe.

## Build System & Tooling

**Decision**: CMake for build, clang‑format/clang‑tidy for code style, vcpkg or system packages for dependencies.

**Rationale**: CMake is widely supported; clang‑tools enforce consistent formatting and static analysis. Dependency management can be handled via system packages (apt) or vcpkg for reproducibility.

**Alternatives considered**:
- Conan: More powerful but steeper learning curve.
- Manual Makefiles: Hard to maintain.

## Command-line Arguments

**Decision**: Server accepts three command-line arguments:
- `--port <int>`: WebSocket server port (default: 8080)
- `--ample-time <seconds>`: Time to wait for disconnected client before marking folded (default: 30)
- `--removal-timeout <seconds>`: Time after which inactive client is removed (default: 60)

**Rationale**: Follows spec requirement for configurable timeouts via command-line only. Simple, explicit, no configuration files. Using GNU long option style for clarity.

**Alternatives considered**: Using configuration file (rejected due to spec), using environment variables (rejected for simplicity).

## WebSocket Ping/Pong Interval

**Decision**: Server sends ping every 30 seconds; client must respond within 10 seconds or be considered disconnected.

**Rationale**: Standard WebSocket keepalive to detect stale connections. 30 seconds provides balance between network overhead and timely detection.

**Alternatives considered**: No ping (rejected due to need for timeout detection), shorter interval (e.g., 5 seconds) rejected as unnecessary overhead.

## Logging Library

**Decision**: Use Boost.Log (part of Boost) for logging.

**Rationale**: Already using Boost.Beast and Boost.Asio; adding Boost.Log maintains dependency consistency and provides structured logging with severity levels (debug/info/error) as required.

**Alternatives considered**: spdlog (lightweight, header-only) rejected to avoid introducing another dependency; plain std::cout rejected due to lack of severity levels.

## Linting/Formatting Standards

**Decision**: Use clang-format with LLVM style; use clang-tidy for static analysis.

**Rationale**: Widely adopted C++ tooling; LLVM style is readable and consistent. Ensures code quality as per constitution.

**Alternatives considered**: Google style (similar, but LLVM more common), no linting (rejected).

## Minimum Test Coverage Threshold

**Decision**: Aim for 80% line coverage for unit tests, with integration tests covering all user scenarios.

**Rationale**: Reasonable threshold for a server with critical game logic; ensures adequate testing while allowing for rapid iteration.

**Alternatives considered**: 90% (too high for initial development), no threshold (rejected per constitution).

## CLI Interface Consistency

**Decision**: Follow GNU Coding Standards for command-line interface: long options with `--`, optional short options `-p`, `-a`, `-r`.

**Rationale**: Provides familiar, consistent user experience; aligns with common Linux tooling.

**Alternatives considered**: No short options (rejected for convenience), positional arguments (rejected for clarity).

## Monitoring/Observability

**Decision**: Console logging with debug/info/error levels; no external metrics collection initially.

**Rationale**: Satisfies spec requirement for basic console logging; additional observability can be added later if needed.

**Alternatives considered**: Prometheus metrics (overkill for single-table server), structured JSON logging (rejected for simplicity).

## Card Representation

**Decision**: Internal representation as integers 0-51 (rank * 4 + suit) for efficient shuffling and hand evaluation; JSON serialization as two‑character strings (rank + suit, e.g., "Ah").

**Rationale**: Integer representation enables fast lookup for hand ranking (two‑plus‑two algorithm); string representation provides human‑readable messages over WebSocket.

**Alternatives considered**: Struct with rank/suit (more memory), string-only representation (inefficient for evaluation), binary encoding (unnecessary complexity).

## Hand Evaluation

**Decision**: Use lookup table (two-plus-two algorithm) for fast hand ranking.

**Rationale**: Industry standard for poker hand evaluation; provides O(1) performance.

**Alternatives considered**: Brute-force evaluation (slow), external library (unnecessary dependency).

## Summary

All “NEEDS CLARIFICATION” markers from the Technical Context and Constitution Check are now resolved. The chosen technology stack is conservative, well‑supported, and aligns with the project’s simplicity requirements.