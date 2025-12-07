# Implementation Plan: Heads-up NLHE Bot Server

**Branch**: `001-heads-up-nlhe-bots` | **Date**: 2025-12-07 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `/specs/001-heads-up-nlhe-bots/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/commands/plan.md` for the execution workflow.

## Summary

Build a C++ server hosting a single heads-up No Limit Texas Hold'em table for two WebSocket clients, implementing standard NLHE rules with configurable timeouts, random-strategy bots, automatic stack top-up, and graceful disconnection handling. Technical approach: C++17 with Boost.Beast for WebSocket communication, nlohmann/json for message serialization, Boost.Asio for asynchronous I/O, and Google Test for unit/integration testing.

## Technical Context

<!--
  ACTION REQUIRED: Replace the content in this section with the technical details
  for the project. The structure here is presented in advisory capacity to guide
  the iteration process.
-->

**Language/Version**: C++17  
**Primary Dependencies**: Boost.Beast (WebSocket/HTTP), nlohmann/json, Boost.Asio  
**Storage**: N/A (no persistence)  
**Testing**: Google Test  
**Target Platform**: Linux server  
**Project Type**: single (server application)  
**Performance Goals**: Sub-second latency, 100+ hands/hour  
**Constraints**: Integer bets only (2/4 blinds), max 2 players, configurable timeouts via command-line (--port, --ample-time, --removal-timeout), WebSocket ping/pong interval 30s, logging via Boost.Log, build system CMake  
**Scale/Scope**: Single table, two players

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

**Code Quality Gate**
- Linting/formatting standards: clang-format (LLVM style), clang-tidy for static analysis
- SOLID principles adherence: To be verified during implementation

**Testing Standards Gate**
- TDD required: Yes, tests must be written before implementation
- Unit/integration/contract tests: Google Test framework selected
- Minimum coverage thresholds: 80% line coverage for unit tests

**User Experience Consistency Gate**
- CLI interface consistency: GNU Coding Standards (long options with --, optional short options -p, -a, -r)
- WebSocket API consistency: To be defined in contracts
- Accessibility standards: N/A (no UI)

**Performance Requirements Gate**
- Performance budgets defined: Sub-second latency, 100+ hands/hour
- Monitoring/observability: Console logging with debug/info/error levels via Boost.Log

## Project Structure

### Documentation (this feature)

```text
specs/001-heads-up-nlhe-bots/
├── plan.md              # This file (/speckit.plan command output)
├── research.md          # Phase 0 output (/speckit.plan command)
├── data-model.md        # Phase 1 output (/speckit.plan command)
├── quickstart.md        # Phase 1 output (/speckit.plan command)
├── contracts/           # Phase 1 output (/speckit.plan command)
└── tasks.md             # Phase 2 output (/speckit.tasks command - NOT created by /speckit.plan)
```

### Source Code (repository root)
<!--
  ACTION REQUIRED: Replace the placeholder tree below with the concrete layout
  for this feature. Delete unused options and expand the chosen structure with
  real paths (e.g., apps/admin, packages/something). The delivered plan must
  not include Option labels.
-->

```text
src/
├── core/           # Poker logic (hand evaluation, deck, betting)
├── server/         # WebSocket server, table management
├── client/         # Bot client logic
└── shared/         # Common utilities (logging, config parsing)

tests/
├── unit/           # Unit tests for core, server, client
├── integration/    # Integration tests (server-client)
└── contract/       # Contract tests for WebSocket API
```

**Structure Decision**: Single project structure with `src/` and `tests/` directories as shown above. This follows the standard C++ project layout and aligns with the existing AGENTS.md guidance.

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| [e.g., 4th project] | [current need] | [why 3 projects insufficient] |
| [e.g., Repository pattern] | [specific problem] | [why direct DB access insufficient] |
