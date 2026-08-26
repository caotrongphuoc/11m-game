# Eleven Meter object sequences

This document describes state ownership and message flow between the Eleven Meter gameplay tasks. Each task owns its mutable state inside its `.cpp` file. Tasks communicate through AK signals and common-message payloads; no gameplay task reads another task's mutable state directly.

## Object responsibilities

| Object | Owned state | Main responsibility |
|---|---|---|
| Match | Match phase, difficulty, countdown, best score, scoreboard | Coordinate menu, rounds, timing, and final result. |
| Shooter | Kick direction, animation frame, reaction timer, random seed | Process the selected kick and dispatch Ball/Keeper commands. |
| Ball | Position, animation frame, movement target | Animate the shot and report its arrival to Goal. |
| Keeper | Dive direction, difficulty, position, animation frame, random seed | Select and animate the goalkeeper response. |
| Goal | Latest Ball and Keeper reports, readiness flags | Resolve goal, save, or miss after both reports arrive. |
| Display | Read-only snapshots received from gameplay tasks | Compose and render the current OLED view. |

## Start a round

```mermaid
sequenceDiagram
    participant Display
    participant Match
    participant Shooter
    participant Ball
    participant Keeper
    participant Goal

    Display->>Match: EM_GAME_MATCH_START
    Match->>Match: EM_GAME_MATCH_SETUP
    Match->>Match: EM_GAME_MATCH_START_ROUND
    Match->>Display: EM_GAME_DISPLAY_SHOW_PENALTY
    Display->>Goal: EM_GAME_GOAL_SETUP
    Display->>Ball: EM_GAME_BALL_SETUP
    Display->>Keeper: EM_GAME_KEEPER_SETUP
    Display->>Shooter: EM_GAME_SHOOTER_SETUP
    Match->>Display: EM_GAME_DISPLAY_UPDATE_MATCH
    Ball->>Display: EM_GAME_DISPLAY_UPDATE_BALL
    Keeper->>Display: EM_GAME_DISPLAY_UPDATE_KEEPER
    Shooter->>Display: EM_GAME_DISPLAY_UPDATE_SHOOTER
```

The penalty screen is entered once when a match starts. Later rounds stay on the same screen;
Match posts `RESET` to Goal, Ball, Keeper, and Shooter before starting the next round.

## Shared game loop

```mermaid
sequenceDiagram
    participant Penalty as Penalty screen
    participant Match
    participant Shooter
    participant Ball
    participant Keeper

    loop Every EM_GAME_TIME_TICK_INTERVAL
        Penalty->>Match: EM_GAME_MATCH_UPDATE
        Penalty->>Shooter: EM_GAME_SHOOTER_UPDATE
        Penalty->>Ball: EM_GAME_BALL_UPDATE
        Penalty->>Keeper: EM_GAME_KEEPER_UPDATE
    end
```

Match advances countdown and result timing from this tick. Animated objects use local elapsed
accumulators, so Ball keeps its 50 ms update interval while Shooter and Keeper keep their 80 ms
intervals. The shared periodic timer is stopped when Match leaves gameplay.

## Select and execute a kick

The screen sends the player's input to Match first. Match accepts it only while the round is in `EM_GAME_MATCH_STATE_SHOOTER_WAIT`, clears the countdown, and forwards one object-specific request to Shooter.

```mermaid
sequenceDiagram
    participant Display
    participant Match
    participant Shooter
    participant Ball
    participant Keeper

    alt Up button
        Display->>Match: EM_GAME_MATCH_KICK_LEFT
        Match->>Shooter: EM_GAME_SHOOTER_REQUEST_KICK_LEFT
    else Mode button
        Display->>Match: EM_GAME_MATCH_KICK_CENTER
        Match->>Shooter: EM_GAME_SHOOTER_REQUEST_KICK_CENTER
    else Down button
        Display->>Match: EM_GAME_MATCH_KICK_RIGHT
        Match->>Shooter: EM_GAME_SHOOTER_REQUEST_KICK_RIGHT
    else Selection timeout
        Match->>Shooter: EM_GAME_SHOOTER_REQUEST_KICK_CENTER
    end

    Shooter->>Shooter: Select Ball target and start animation
    Shooter->>Ball: EM_GAME_BALL_KICK<br/>em_game_ball_kick_t
    Shooter->>Keeper: EM_GAME_KEEPER_REACT<br/>em_game_keeper_react_t
    Shooter->>Display: EM_GAME_DISPLAY_UPDATE_SHOOTER
    Ball->>Display: EM_GAME_DISPLAY_UPDATE_BALL
    Keeper->>Display: EM_GAME_DISPLAY_UPDATE_KEEPER
```

Shooter owns the player's kick direction. Ball receives only its movement target, while Keeper receives only the shot zone it needs to choose a dive.

## Resolve the kick

Goal does not inspect Ball or Keeper state. It waits for one message from each object and resolves the attempt only when both reports are ready.

```mermaid
sequenceDiagram
    participant Ball
    participant Keeper
    participant Goal
    participant Match
    participant Display

    Ball->>Ball: Complete movement
    Ball->>Goal: EM_GAME_GOAL_BALL_ARRIVED<br/>em_game_goal_ball_t
    Keeper->>Keeper: Complete dive
    Keeper->>Goal: EM_GAME_GOAL_KEEPER_READY<br/>em_game_goal_keeper_t

    Goal->>Goal: Validate both payloads
    Goal->>Goal: Resolve GOAL / SAVE / MISS
    Goal->>Match: EM_GAME_MATCH_HIT_RESULT<br/>em_game_goal_result_msg_t
    Match->>Match: Update scoreboard
    Match->>Display: EM_GAME_DISPLAY_UPDATE_MATCH
    Display->>Match: EM_GAME_MATCH_UPDATE
```

The order of the Ball and Keeper arrival messages is not significant. Goal stores the first valid report and waits for the other one.

While the penalty screen is active, its periodic `EM_GAME_TIME_TICK` drives Match and the
animated gameplay objects without blocking a task handler.

## Display snapshots

Gameplay objects send read-only snapshots to Display whenever their visible state changes:

| Sender | Signal | Payload |
|---|---|---|
| Match | `EM_GAME_DISPLAY_UPDATE_MATCH` | `em_game_match_view_t` |
| Shooter | `EM_GAME_DISPLAY_UPDATE_SHOOTER` | `em_game_shooter_view_t` |
| Ball | `EM_GAME_DISPLAY_UPDATE_BALL` | `em_game_ball_view_t` |
| Keeper | `EM_GAME_DISPLAY_UPDATE_KEEPER` | `em_game_keeper_view_t` |

All payload structures are checked at compile time against `AK_COMMON_MSG_DATA_SIZE`. Display validates the received payload length before updating its local render snapshot.
