# WebSocket API Contract

## Connection Establishment

1. Client opens a WebSocket connection to `ws://server:port/table`.
2. Server sends a `welcome` message with assigned player ID and current table state.
3. Client must send a `join` message to take a seat (if empty).

## Message Format

All messages are JSON objects with a `type` field and a `payload` field.

```json
{
  "type": "message_type",
  "payload": { ... }
}
```

## Server → Client Messages

### `welcome`
Sent immediately after connection.

```json
{
  "type": "welcome",
  "payload": {
    "player_id": "uuid",
    "table": {
      "seat_1": { "player_id": "uuid", "stack": 400, "connected": true },
      "seat_2": { "player_id": "uuid", "stack": 400, "connected": true },
      "current_hand": null,
      "pot": 0,
      "community_cards": [],
      "dealer_button_position": 0
    }
  }
}
```

### `hand_started`
Indicates a new hand has begun.

```json
{
  "type": "hand_started",
  "payload": {
    "hand_id": "uuid",
    "players": [
      { "player_id": "uuid", "stack": 400, "hole_cards": ["Ah", "Kd"] },
      { "player_id": "uuid", "stack": 400, "hole_cards": ["Qs", "Jc"] }
    ],
    "small_blind": 2,
    "big_blind": 4,
    "dealer_position": 0,
    "current_player_to_act": "uuid",
    "min_raise": 4
  }
}
```

### `action_request`
Sent to the player whose turn it is.

```json
{
  "type": "action_request",
  "payload": {
    "hand_id": "uuid",
    "possible_actions": ["fold", "call", "raise"],
    "call_amount": 4,
    "min_raise": 8,
    "max_raise": 400,
    "timeout_ms": 30000
  }
}
```

### `action_applied`
Broadcast to all players after a player acts.

```json
{
  "type": "action_applied",
  "payload": {
    "hand_id": "uuid",
    "player_id": "uuid",
    "action": "raise",
    "amount": 20,
    "new_stack": 380,
    "pot": 44,
    "next_player_to_act": "uuid"
  }
}
```

### `community_cards_dealt`
Broadcast when new community cards are revealed.

```json
{
  "type": "community_cards_dealt",
  "payload": {
    "hand_id": "uuid",
    "round": "flop",
    "cards": ["2h", "5d", "9c"],
    "pot": 100,
    "next_player_to_act": "uuid"
  }
}
```

### `hand_completed`
Sent when hand ends (showdown or fold).

```json
{
  "type": "hand_completed",
  "payload": {
    "hand_id": "uuid",
    "winners": [
      { "player_id": "uuid", "amount_won": 200, "hand_rank": "two_pair" }
    ],
    "pot_distribution": [
      { "pot_index": 0, "winner_id": "uuid", "amount": 200 }
    ],
    "updated_stacks": {
      "uuid": 600,
      "uuid": 200
    }
  }
}
```

### `player_disconnected`
Broadcast when a player loses connection.

```json
{
  "type": "player_disconnected",
  "payload": {
    "player_id": "uuid",
    "remaining_grace_time_ms": 30000
  }
}
```

### `player_reconnected`
Broadcast when a player re‑joins within grace period.

```json
{
  "type": "player_reconnected",
  "payload": {
    "player_id": "uuid"
  }
}
```

### `player_removed`
Broadcast when a player is removed after timeout.

```json
{
  "type": "player_removed",
  "payload": {
    "player_id": "uuid",
    "seat": 0
  }
}
```

### `error`
Sent for invalid messages or conditions.

```json
{
  "type": "error",
  "payload": {
    "code": "invalid_bet",
    "message": "Bet must be an integer between 4 and your stack."
  }
}
```

## Client → Server Messages

### `join`
Client requests to take a seat (if empty).

```json
{
  "type": "join",
  "payload": {
    "name": "BotAlice"
  }
}
```

### `action`
Client response to `action_request`.

```json
{
  "type": "action",
  "payload": {
    "hand_id": "uuid",
    "action": "raise",
    "amount": 20
  }
}
```

### `ping`
Keep‑alive (optional). Server responds with `pong`.

```json
{
  "type": "ping",
  "payload": {}
}
```

## Error Handling

- If a client sends an invalid message, the server replies with an `error` message and may close the connection.
- If a client sends an `action` out‑of‑turn, it is ignored (or error).
- If a client disconnects, the server starts the grace timer; if the client re‑connects with the same player ID (e.g., via session token), it resumes its seat.

## Timeouts

- **Action timeout**: 30 seconds per player (configurable). If no action, player is folded.
- **Disconnection grace time**: 30 seconds (configurable) before being marked folded.
- **Removal timeout**: 1 minute (configurable) before being removed from the table.

## Example Flow

1. Client connects → receives `welcome`.
2. Client sends `join` → server assigns seat.
3. When two players are joined, server starts hand → `hand_started`.
4. Server sends `action_request` to first player.
5. Client responds with `action` → server broadcasts `action_applied`.
6. Repeat until hand ends → `hand_completed`.
7. Begin next hand.