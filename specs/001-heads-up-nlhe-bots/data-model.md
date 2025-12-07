# Data Model

## Entities

### Table
Represents a single heads‑up poker table.

**Fields**:
- `id`: unique identifier (UUID or integer)
- `seat_1`: reference to Player in seat 1 (button)
- `seat_2`: reference to Player in seat 2 (big blind)
- `current_hand`: reference to Hand currently being played (nullable)
- `pot`: total chips in the main pot (integer)
- `side_pots`: list of side pots (if any)
- `community_cards`: array of 0–5 cards (string representation, e.g., "Ah" for Ace of hearts)
- `dealer_button_position`: which seat is the dealer (0 or 1)
- `state`: enum { WAITING_FOR_PLAYERS, HAND_IN_PROGRESS, HAND_COMPLETE }

**Validation**:
- Exactly two seats.
- Blinds are fixed at 2/4 (small blind = 2, big blind = 4).
- Bets must be integers.

### Player
Represents a connected client.

**Fields**:
- `id`: unique identifier (UUID or integer)
- `name`: optional display name
- `stack`: current chip count (integer)
- `seat`: which seat at the table (0 or 1)
- `hole_cards`: array of exactly 2 cards (string representation) when in a hand
- `connection_status`: enum { CONNECTED, DISCONNECTED, RECONNECTING }
- `last_action_timestamp`: last time the player acted (for timeout detection)
- `disconnected_at`: timestamp when connection lost (nullable)
- `is_sitting_out`: boolean (true if player has been folded due to timeout)

**Validation**:
- Stack cannot be negative.
- Hole cards are private and only visible to the player.
- Stack top‑up occurs between hands when stack < 5BB (20 chips).

### Hand
Represents a single poker hand from deal to showdown.

**Fields**:
- `id`: unique identifier (UUID or integer)
- `table_id`: reference to the Table
- `players`: list of Player references participating in the hand
- `deck`: reference to Deck (shuffled)
- `community_cards`: array of 0–5 cards (copied to Table for convenience)
- `pot`: total chips in the main pot (copied from Table)
- `side_pots`: list of side pots (copied from Table)
- `current_betting_round`: enum { PREFLOP, FLOP, TURN, RIVER, SHOWDOWN }
- `current_player_to_act`: reference to Player whose turn it is
- `min_raise`: minimum raise amount (based on previous bet/raise)
- `history`: list of actions (player, action, amount, timestamp)
- `winners`: list of Player references who won (populated at showdown)
- `completed_at`: timestamp when hand finished

**Validation**:
- Betting rounds follow standard NLHE order.
- Minimum raise is at least the big blind (4) after a bet/raise.
- All‑in situations create side pots.

### Deck
Standard 52‑card deck.

**Fields**:
- `cards`: array of 52 card strings (e.g., ["Ah", "Kh", ...])
- `next_card_index`: integer pointing to the next card to deal

**Validation**:
- Cards are unique.
- Deck is shuffled before each hand.

## Relationships

- A **Table** has exactly two **Player** seats (one‑to‑two).
- A **Table** has zero or one active **Hand** (one‑to‑zero/one).
- A **Hand** belongs to one **Table** (many‑to‑one).
- A **Hand** involves two **Player**s (many‑to‑many).
- A **Hand** uses one **Deck** (one‑to‑one).
- A **Player** can be seated at one **Table** (one‑to‑one).

## State Transitions

### Table State Machine
1. `WAITING_FOR_PLAYERS` → `HAND_IN_PROGRESS` when two players are connected and seated.
2. `HAND_IN_PROGRESS` → `HAND_COMPLETE` when hand reaches showdown or all but one player folds.
3. `HAND_COMPLETE` → `WAITING_FOR_PLAYERS` (or `HAND_IN_PROGRESS` for next hand) after pot distribution and stack top‑up.

### Player Connection State
1. `CONNECTED` → `DISCONNECTED` on network loss.
2. `DISCONNECTED` → `RECONNECTING` if client re‑connects within ample time (30 seconds).
3. `RECONNECTING` → `CONNECTED` on successful re‑join.
4. `DISCONNECTED` → `SITTING_OUT` after ample time expires (player is folded).
5. `SITTING_OUT` → `REMOVED` after removal timeout (1 minute) and seat becomes empty.

### Hand Betting Rounds
1. `PREFLOP` → `FLOP` after all players have acted and no more betting possible.
2. `FLOP` → `TURN` (same condition).
3. `TURN` → `RIVER` (same condition).
4. `RIVER` → `SHOWDOWN` (same condition).
5. Any round can transition to `SHOWDOWN` if all but one player folds.

## Validation Rules (from Requirements)

- **FR‑002**: Standard NLHE rules enforced by the Hand state machine.
- **FR‑003**: Bets must be integers, minimum bet = big blind (4), maximum bet = player's stack.
- **FR‑004**: Disconnection handling: ample time (configurable, default 30 seconds) before marking player folded.
- **FR‑005**: Removal timeout (configurable, default 1 minute) before removing inactive client.
- **FR‑006**: Player stack starts at 100BB (400 chips at 2/4 blinds).
- **FR‑008**: Stack top‑up when stack < 5BB (20 chips) between hands, not during a hand.
- **FR‑009**: Random delay of 0.5–3 seconds before each bot action.
- **FR‑010**: Server must handle malformed messages gracefully (return error, disconnect abusive clients).

## Data Representation

- Cards: two‑character string: rank (A, K, Q, J, T, 9–2) + suit (h, d, c, s). Example: "Ah" = Ace of hearts.
- Chip amounts: integers (no fractional chips).
- Timestamps: UNIX epoch milliseconds.
- Enums: represented as strings in JSON (e.g., "PREFLOP").

## Persistence

No persistent storage required; all state is held in memory. If server restarts, the game resets.