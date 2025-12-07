# Feature Specification: Heads-up NLHE Bot Server

**Feature Branch**: `001-heads-up-nlhe-bots`  
**Created**: Sun Dec 07 2025  
**Status**: Draft  
**Input**: User description: "Build a server that will host 2 clients that will play heads up nlhe. It will use the standard NLHE rules. Handle timeouts gracefully. Build a client that will connect to the server and will start with 100BB. It will be a bot playing with a random strategy. If the stack goes below 5BB, it will automatically top up to 100BB. It will have random delay like a human player. If the client is disconnected, it will be given ample time to return, otherwise it will be considered folded and sat out. After a while it will be removed from the table if it does not return. The server must handle everything gracefully. The server will only accommodate 2 players. So it basically has only 1 table. In order to simplify, the blinds will be at 2/4, and bets must be integers."

## Clarifications

### Session 2025-12-07

- Q: How should split pots (tied hands) be handled? → A: Divide pot equally among winners, with odd chips given to player in earliest position (button order).
- Q: How should server configuration (timeouts, blinds, etc.) be provided? → A: Command-line arguments only (simple, explicit).
- Q: How should the server handle crashes mid-hand? → A: Game resets on restart (no persistence).
- Q: How should the server handle invalid bets (non-integer, negative, or exceeding stack)? → A: Exceeding stack treated as all-in; negative/invalid treated as check/fold.
- Q: How should the server handle raise amounts that exceed opponent's stack? → A: Treat as all-in (opponent can call with remaining stack).

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Server hosts NLHE game between two bots (Priority: P1)

As an operator, I want to start a server that hosts a heads-up No Limit Texas Hold'em table, so that two bot clients can connect and play a complete hand using standard NLHE rules.

**Why this priority**: This is the core functionality - without a working server and basic game rules, no poker can be played.

**Independent Test**: Can be tested by starting the server, connecting two bot clients, and verifying they can play through a full hand (pre-flop, flop, turn, river) with proper betting rounds and pot distribution.

**Acceptance Scenarios**:

1. **Given** the server is running with an empty table, **When** two bot clients connect, **Then** the server starts a new hand with blinds posted and deals hole cards.
2. **Given** a hand in progress, **When** betting rounds complete, **Then** the server correctly evaluates winners and awards the pot.
3. **Given** a player is all-in, **When** side pots are created, **Then** the server distributes chips correctly.

---

### User Story 2 - Bots play with realistic behavior and stack management (Priority: P2)

As an operator, I want bots to play with a random strategy, include human-like delays, and automatically top up their stacks when low, so that the game simulates realistic poker play.

**Why this priority**: Adds realism and ensures the game continues without manual intervention when stacks get low.

**Independent Test**: Can be tested by running a session where bots make random decisions (fold, call, raise) with variable delays, and observing that stacks are topped up to 100BB when they fall below 5BB.

**Acceptance Scenarios**:

1. **Given** a bot with stack below 5BB, **When** the hand ends, **Then** the bot's stack is automatically topped up to 100BB.
2. **Given** a bot's turn to act, **When** the bot makes a decision, **Then** there is a random delay (0.5-3 seconds) before the action is sent.
3. **Given** multiple betting options, **When** the bot acts, **Then** it chooses randomly between valid actions (fold, call, raise) with reasonable raise sizes.

---

### User Story 3 - Server handles disconnections and timeouts gracefully (Priority: P3)

As an operator, I want the server to handle client disconnections gracefully by giving ample time to reconnect, and eventually removing inactive players, so that the game remains stable and fair.

**Why this priority**: Ensures system robustness and prevents games from hanging due to network issues.

**Independent Test**: Can be tested by disconnecting a client during a hand and verifying it's given time to reconnect before being marked folded, and eventually removed if it doesn't return.

**Acceptance Scenarios**:

1. **Given** a connected client, **When** the client disconnects during a hand, **Then** the server waits for a configurable "ample time" (e.g., 30 seconds) before marking the player folded.
2. **Given** a disconnected client, **When** the client does not reconnect within a removal timeout (e.g., 5 minutes), **Then** the server removes the client from the table.
3. **Given** a client that reconnects within the ample time, **When** it rejoins, **Then** it resumes play with its current stack and position.

---

### Edge Cases

- What happens when both bots disconnect simultaneously?
- Invalid bets: exceeding stack treated as all-in; negative/non-integer treated as check/fold.
- Raise amounts exceeding opponent's stack are treated as all-in (opponent can call with remaining stack).
- How does the system handle network latency and packet loss?
- Server crashes mid-hand result in game reset on restart (no persistence).
- What if a bot tops up during a hand? (Should only happen between hands)
- Tied hands (split pots) are handled by dividing pot equally among winners, with odd chips given to player in earliest position (button order).

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: Server MUST host exactly one table for exactly two players (heads-up).
- **FR-002**: Server MUST implement standard No Limit Texas Hold'em rules (blinds, four betting rounds, hand ranking, pot distribution).
- **FR-003**: Server MUST enforce integer bets with minimum bet equal to big blind (4) and maximum bet up to a player's stack.
- **FR-004**: Server MUST handle client disconnections by waiting for a configurable "ample time" (default: 30 seconds) before marking the player folded and sitting them out.
- **FR-005**: Server MUST remove inactive clients after a configurable "removal timeout" (default: 1 minute) if they do not reconnect.
- **FR-006**: Client MUST connect to server and start with 100BB stack (400 chips at 2/4 blinds).
- **FR-007**: Client MUST play with a random strategy, choosing randomly between valid actions (fold, call, raise) with raise amounts between minimum raise and player's stack.
- **FR-008**: Client MUST automatically top up stack to 100BB when stack falls below 5BB (between hands, not during a hand).
- **FR-009**: Client MUST introduce random delay between 0.5 and 3 seconds before each action to simulate human play.
- **FR-010**: Server MUST handle network timeouts and malformed messages gracefully without crashing.
- **FR-011**: Server configuration (port, timeouts, etc.) MUST be provided via command-line arguments.
- **FR-012**: Server MUST handle invalid bets appropriately: bets exceeding player's stack treated as all-in; negative or non-integer bets treated as check or fold.
- **FR-013**: Server MUST handle raise amounts exceeding opponent's stack as all-in bets (opponent can call with remaining stack).

### Key Entities *(include if feature involves data)*

- **Table**: Represents a single heads-up poker table with two seats, blinds (2/4), pot, and current hand state.
- **Player**: Represents a connected client with a stack size, position (button/blind), hole cards, and connection status.
- **Hand**: Represents a single poker hand including deck, community cards, betting rounds, pot(s), and winner(s).
- **Deck**: Standard 52-card deck used for dealing hole cards and community cards.

### Assumptions

- The server and clients will run in a controlled environment (e.g., local network or same machine).
- Network connectivity is generally reliable; occasional disconnections are expected.
- No authentication or security is required beyond basic connection acceptance.
- The random number generator used for bot decisions does not need to be cryptographically secure.
- The server does not need to persist game state across restarts (optional but not required).
- The "standard NLHE rules" refer to typical casino rules for heads-up No Limit Hold'em (button posts small blind, big blind is posted by the other player).

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: Two bots can play at least 100 consecutive hands without manual intervention (excluding disconnections).
- **SC-002**: Server maintains game state correctly across hand transitions with no chip leaks or state corruption (total chips in play remains constant).
- **SC-003**: Disconnected clients are given configurable wait time (default 30 seconds) before being marked folded and sat out.
- **SC-004**: Inactive clients are removed after configurable timeout (default 1 minute) if they do not reconnect, freeing up their seat.
- **SC-005**: Bots exhibit random decision-making with realistic delays (0.5-3 seconds per action) across 95% of actions.
- **SC-006**: Stack top-up automatically occurs when a bot's stack falls below 5BB, restoring it to 100BB between hands.
- **SC-007**: Server handles invalid inputs and network errors gracefully without crashing or requiring restart.
