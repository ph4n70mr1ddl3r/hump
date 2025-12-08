# Tasks: 001-heads-up-nlhe-bots

**Input**: Design documents from `/specs/001-heads-up-nlhe-bots/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/
**Tests**: TDD required per constitution - all test tasks included.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and basic structure

- [X] T001 Create project structure per implementation plan (src/core/, src/server/, src/client/, src/common/, tests/unit/, tests/integration/, tests/contract/, cmake/, scripts/)
- [X] T002 Initialize CMake project with dependencies (Boost.Beast, nlohmann/json, Google Test) in CMakeLists.txt
- [X] T003 [P] Configure clang-format and clang-tidy in .clang-format and .clang-tidy

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

### Tests for Foundational Components (TDD)

- [X] T004 [P] Write unit tests for card representation and deck shuffling in tests/unit/core/deck_test.cpp
- [X] T005 [P] Write unit tests for hand ranking logic in tests/unit/core/hand_ranking_test.cpp
- [X] T006 [P] Write unit tests for betting rules in tests/unit/core/betting_rules_test.cpp
- [X] T007 [P] Write unit tests for JSON serialization in tests/unit/common/json_serialization_test.cpp
- [X] T008 [P] Write integration test for WebSocket connection in tests/integration/websocket_connection_test.cpp
- [X] T008b [P] Contract test for malformed messages and invalid inputs in tests/contract/malformed_messages_test.cpp
### Implementation for Foundational Components

- [X] T009 [P] Implement card representation and deck shuffling in src/core/card.cpp and src/core/card.hpp
- [X] T010 [P] Implement hand ranking logic in src/core/hand_ranking.cpp and src/core/hand_ranking.hpp
- [X] T011 [P] Implement betting rules (minimum raise, valid actions) in src/core/betting_rules.cpp and src/core/betting_rules.hpp
- [X] T012 [P] Implement JSON serialization for core types and WebSocket message types in src/common/json_serialization.cpp and src/common/json_serialization.hpp
- [X] T013 [P] Implement basic WebSocket server skeleton (accept connections, echo) in src/server/server.cpp and src/server/server.hpp
- [X] T014 [P] Implement basic bot client skeleton (connect, ping) with command-line argument parsing in src/client/client.cpp and src/client/client.hpp
- [X] T015 [P] Create data model structs (Table, Player, Hand, Deck) in src/core/models/ (table.hpp, player.hpp, hand.hpp, deck.hpp)
- [X] T016 Implement common utilities (logging, UUID generation) in src/common/logging.cpp and src/common/uuid.cpp

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - Server hosts NLHE game between two bots (Priority: P1) 🎯 MVP

**Goal**: Start a server that hosts a heads-up No Limit Texas Hold'em table, two bot clients can connect and play a complete hand using standard NLHE rules.

**Independent Test**: Can be tested by starting the server, connecting two bot clients, and verifying they can play through a full hand (pre-flop, flop, turn, river) with proper betting rounds and pot distribution.

### Tests for User Story 1 (TDD)

- [X] T017 [P] [US1] Contract test for welcome message in tests/contract/welcome_test.cpp
- [X] T018 [P] [US1] Contract test for hand_started message in tests/contract/hand_started_test.cpp
- [X] T019 [P] [US1] Contract test for action_request and action_applied messages in tests/contract/action_messages_test.cpp
- [X] T020 [P] [US1] Contract test for hand_completed message in tests/contract/hand_completed_test.cpp
- [X] T021 [P] [US1] Integration test for full hand flow in tests/integration/full_hand_test.cpp

### Implementation for User Story 1

- [X] T022 [P] [US1] Implement Table management (seat assignment, hand start) in src/server/table_manager.cpp and src/server/table_manager.hpp
- [X] T023 [P] [US1] Implement Hand state machine (betting rounds, community cards) in src/core/hand.cpp and src/core/hand.hpp
- [X] T024 [US1] Implement Player action processing (fold, call, raise) in src/server/player_action.cpp and src/server/player_action.hpp
- [X] T025 [US1] Implement pot calculation and distribution in src/core/pot.cpp and src/core/pot.hpp
- [X] T026 [US1] Integrate WebSocket server with game logic (message handling) in src/server/game_session.cpp and src/server/game_session.hpp
- [X] T027 [US1] Implement server command line arguments (port, config) in src/server/main.cpp
- [X] T028 [US1] Implement client join flow (welcome, join messages) in src/client/bot.cpp

**Checkpoint**: At this point, User Story 1 should be fully functional and testable independently

---

## Phase 4: User Story 2 - Bots play with realistic behavior and stack management (Priority: P2)

**Goal**: Bots play with a random strategy, include human-like delays, and automatically top up their stacks when low.

**Independent Test**: Can be tested by running a session where bots make random decisions (fold, call, raise) with variable delays, and observing that stacks are topped up to 100BB when they fall below 5BB.

### Tests for User Story 2 (TDD)

- [X] T029 [P] [US2] Unit test for random decision logic in tests/unit/client/random_strategy_test.cpp
- [X] T030 [P] [US2] Unit test for stack top-up logic in tests/unit/client/stack_management_test.cpp
- [X] T031 [P] [US2] Integration test for bot behavior with delays in tests/integration/bot_behavior_test.cpp

### Implementation for User Story 2

- [X] T032 [P] [US2] Implement random strategy (fold/call/raise with raise amounts between minimum raise and player's stack) in src/client/random_strategy.cpp and src/client/random_strategy.hpp
- [X] T033 [P] [US2] Implement delay mechanism (0.5-3 seconds) in src/client/delay.cpp and src/client/delay.hpp
- [X] T034 [US2] Implement stack top-up detection and execution (between hands) in src/client/stack_management.cpp and src/client/stack_management.hpp
- [X] T035 [US2] Integrate random strategy and delays into bot main loop in src/client/bot.cpp

**Checkpoint**: At this point, User Stories 1 AND 2 should both work independently

---

## Phase 5: User Story 3 - Server handles disconnections and timeouts gracefully (Priority: P3)

**Goal**: Server handles client disconnections gracefully by giving ample time to reconnect, and eventually removing inactive players.

**Independent Test**: Can be tested by disconnecting a client during a hand and verifying it's given time to reconnect before being marked folded, and eventually removed if it doesn't return.

### Tests for User Story 3 (TDD)

- [X] T036 [P] [US3] Unit test for disconnection timer logic in tests/unit/server/disconnection_timer_test.cpp
- [X] T037 [P] [US3] Integration test for disconnection and reconnection flow in tests/integration/disconnection_test.cpp

### Implementation for User Story 3

- [X] T038 [P] [US3] Implement disconnection detection and grace timer in src/server/connection_manager.cpp and src/server/connection_manager.hpp
- [X] T039 [P] [US3] Implement player state management for disconnected/reconnecting players in src/server/player_state.cpp and src/server/player_state.hpp
- [X] T040 [US3] Implement removal timeout logic (remove inactive players) in src/server/player_removal.cpp and src/server/player_removal.hpp
- [X] T041 [US3] Implement WebSocket ping/pong keep-alive and timeout handling in src/server/websocket_session.cpp

**Checkpoint**: All user stories should now be independently functional

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories

- [X] T042 [P] Documentation updates in README.md and quickstart.md validation
- [X] T043 [P] Code cleanup and refactoring (ensure consistent style)
- [X] T044 [P] Performance optimization across all stories (latency measurements)
- [X] T045 [P] Additional unit tests for edge cases in tests/unit/edge_cases/ (see spec.md:58-66: simultaneous disconnection, invalid bets, raise exceeding stack, network latency, server crash, top-up during hand, split pots)
- [X] T045b [P] Integration test for 100 consecutive hands stability (SC-001) in tests/integration/stability_test.cpp
- [X] T046 Security hardening (input validation, message sanitization)
- [ ] T047 Run quickstart.md validation end-to-end (cancelled due to time)

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - BLOCKS all user stories
- **User Stories (Phase 3+)**: All depend on Foundational phase completion
  - User stories can then proceed in parallel (if staffed)
  - Or sequentially in priority order (P1 → P2 → P3)
- **Polish (Final Phase)**: Depends on all desired user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational (Phase 2) - No dependencies on other stories
- **User Story 2 (P2)**: Can start after Foundational (Phase 2) - Depends on US1 for game flow but can be tested independently with mock server
- **User Story 3 (P3)**: Can start after Foundational (Phase 2) - Depends on US1 for game flow but can be tested independently with mock server

### Within Each User Story

- Tests (if included) MUST be written and FAIL before implementation
- Models before services
- Services before endpoints
- Core implementation before integration
- Story complete before moving to next priority

### Parallel Opportunities

- All Setup tasks marked [P] can run in parallel
- All Foundational tasks marked [P] can run in parallel (within Phase 2)
- Once Foundational phase completes, all user stories can start in parallel (if team capacity allows)
- All tests for a user story marked [P] can run in parallel
- Models within a story marked [P] can run in parallel
- Different user stories can be worked on in parallel by different team members

---

## Parallel Example: User Story 1

```bash
# Launch all tests for User Story 1 together:
Task: "Contract test for welcome message in tests/contract/welcome_test.cpp"
Task: "Contract test for hand_started message in tests/contract/hand_started_test.cpp"
Task: "Contract test for action_request and action_applied messages in tests/contract/action_messages_test.cpp"
Task: "Contract test for hand_completed message in tests/contract/hand_completed_test.cpp"
Task: "Integration test for full hand flow in tests/integration/full_hand_test.cpp"

# Launch all implementation tasks for User Story 1 together:
Task: "Implement Table management in src/server/table_manager.cpp"
Task: "Implement Hand state machine in src/core/hand.cpp"
Task: "Implement Player action processing in src/server/player_action.cpp"
Task: "Implement pot calculation in src/core/pot.cpp"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational (CRITICAL - blocks all stories)
3. Complete Phase 3: User Story 1
4. **STOP and VALIDATE**: Test User Story 1 independently
5. Deploy/demo if ready

### Incremental Delivery

1. Complete Setup + Foundational → Foundation ready
2. Add User Story 1 → Test independently → Deploy/Demo (MVP!)
3. Add User Story 2 → Test independently → Deploy/Demo
4. Add User Story 3 → Test independently → Deploy/Demo
5. Each story adds value without breaking previous stories

### Parallel Team Strategy

With multiple developers:

1. Team completes Setup + Foundational together
2. Once Foundational is done:
   - Developer A: User Story 1
   - Developer B: User Story 2
   - Developer C: User Story 3
3. Stories complete and integrate independently

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Each user story should be independently completable and testable
- Verify tests fail before implementing
- Commit after each task or logical group
- Stop at any checkpoint to validate story independently
- Avoid: vague tasks, same file conflicts, cross-story dependencies that break independence