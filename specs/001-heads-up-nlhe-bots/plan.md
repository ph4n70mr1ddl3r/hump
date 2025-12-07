# Implementation Plan: Heads-up NLHE Bot Server

**Branch**: `001-heads-up-nlhe-bots` | **Date**: Sun Dec 07 2025 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `/specs/[###-feature-name]/spec.md`

**Note**: This template is filled in by the `/speckit.plan` command. See `.specify/templates/commands/plan.md` for the execution workflow.

## Summary

Implement a server hosting a single heads-up No Limit Texas Hold'em table for two bot clients. Server uses C++17 with Boost.Beast for WebSocket communication, nlohmann/json for message serialization, and Google Test for testing. Bots play with random strategy, human-like delays, and automatic stack top-up. Server handles disconnections gracefully with configurable timeouts.

## Technical Context

<!--
  ACTION REQUIRED: Replace the content in this section with the technical details
  for the project. The structure here is presented in advisory capacity to guide
  the iteration process.
-->

**Language/Version**: C++17  
**Primary Dependencies**: Boost.Beast (WebSocket/HTTP), nlohmann/json, Boost.Asio  
**Storage**: N/A (no persistence)  
**Testing**: Google Test (gtest)  
**Target Platform**: Linux server
**Project Type**: single (server)  
**Performance Goals**: Handle up to 2 concurrent connections with sub-second response times; typical poker hand latency < 1 second per action.  
**Constraints**: Must handle client disconnections with configurable timeouts; bets must be integers; no persistence required.  
**Scale/Scope**: Single table, 2 players, no scaling required.

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

### I. Code Quality
- **Requirement**: All code MUST adhere to established linting and formatting standards.
- **Compliance**: Plan includes clang‑format/clang‑tidy; C++17 standard conventions followed.
- **Status**: ✅ PASS

### II. Testing Standards
- **Requirement**: TDD REQUIRED for all new features; unit, integration, contract tests maintained.
- **Compliance**: Google Test (gtest) selected; poker logic will be unit‑tested; integration tests for WebSocket communication.
- **Status**: ✅ PASS

### III. User Experience Consistency
- **Requirement**: User interfaces MUST maintain consistency across all screens and workflows.
- **Compliance**: Not directly applicable (headless server), but WebSocket API will follow consistent message patterns.
- **Status**: ✅ PASS (with note)

### IV. Performance Requirements
- **Requirement**: Performance budgets MUST be defined and monitored for all user-facing operations.
- **Compliance**: Performance goals defined (sub‑second response times, typical hand latency < 1s). Budgets to be refined during implementation.
- **Status**: ⚠️ PARTIAL (budgets need refinement)

### Development Workflow
- **Requirement**: All features MUST follow specification → plan → tasks workflow.
- **Compliance**: This plan follows `.specify/` templates; spec, plan, tasks phases executed.
- **Status**: ✅ PASS

### Governance
- **Requirement**: Amendments require documentation, approval; complexity MUST be justified.
- **Compliance**: Complexity tracked in “Complexity Tracking” section; any violations justified.
- **Status**: ✅ PASS

## Project Structure

### Documentation (this feature)

```text
specs/[###-feature]/
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
├── core/           # Shared poker logic (hand ranking, betting rules, deck)
├── server/         # WebSocket server, table management, connection handling
├── client/         # Bot client implementation, random strategy, delays
├── common/         # Shared utilities (logging, random number generation, config)
└── third_party/    # Vendored dependencies (if any)

tests/
├── unit/           # Unit tests for core logic
├── integration/    # Integration tests for server‑client communication
└── contract/       # Contract tests for WebSocket API (optional)
```

**Structure Decision**: Single CMake project with multiple executables and a shared core library, as outlined in research.md. The `src/` directory is organized by domain (core, server, client) rather than by technical layer (models, services) to keep related code together and simplify dependencies.

## Complexity Tracking

> **Fill ONLY if Constitution Check has violations that must be justified**

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| [e.g., 4th project] | [current need] | [why 3 projects insufficient] |
| [e.g., Repository pattern] | [specific problem] | [why direct DB access insufficient] |
