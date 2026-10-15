# Eleven Meter runtime signal flow

This document shows the end-to-end AK signal path from button input to rendering. Object-level payload details are documented in [Eleven Meter object sequences](03-design-sequence-object.md).

## Runtime overview

```mermaid
flowchart LR
    Buttons[Board buttons] --> BSP[app_bsp]
    BSP -->|AC_DISPLAY_BUTTON_*| Display[Display task and active screen]
    Display -->|menu or kick input| Match[Match task]
    Display -->|40 ms shared tick| Match
    Display -->|40 ms shared tick| Shooter[Shooter task]
    Display -->|40 ms shared tick| Ball[Ball task]
    Display -->|40 ms shared tick| Keeper[Keeper task]
    Match -->|accepted kick| Shooter
    Shooter -->|ball target| Ball
    Shooter -->|shot zone| Keeper
    Ball -->|arrival report| Goal[Goal task]
    Keeper -->|ready report| Goal
    Goal -->|validated result| Match
    Match -->|match snapshot and screen request| Display
    Shooter -->|view snapshot| Display
    Ball -->|view snapshot| Display
    Keeper -->|view snapshot| Display
```

The penalty screen is the only owner of the periodic gameplay tick. Match owns every screen
transition and removes that tick before requesting the RIP screen.

## Match state lifecycle

```mermaid
stateDiagram-v2
    [*] --> MENU: menu entry / INIT
    MENU --> ROUND_START: START
    ROUND_START --> SHOOTER_WAIT: SETUP + START_ROUND
    SHOOTER_WAIT --> REVEAL: accepted kick or timeout
    REVEAL --> ROUND_END: one valid result
    ROUND_END --> ROUND_START: delay, rounds remain
    ROUND_END --> RIP: delay, five results complete
    RIP --> GAME_OVER: timeout or skip
    GAME_OVER --> ROUND_START: RETRY
    GAME_OVER --> MENU: HOME
```

Input received outside the phase shown for its transition is ignored.

## Button to render sequence

```mermaid
sequenceDiagram
    participant Button as Button ISR/BSP
    participant Display
    participant Match
    participant Shooter
    participant Ball
    participant Keeper
    participant Goal
    participant Buzzer

    Button->>Display: AC_DISPLAY_BUTTON_*_PRESSED
    Display->>Match: EM_GAME_MATCH_KICK_*
    Match->>Shooter: EM_GAME_SHOOTER_REQUEST_KICK_*
    Shooter->>Ball: EM_GAME_BALL_KICK
    Shooter->>Keeper: EM_GAME_KEEPER_REACT

    loop Every 40 ms while penalty screen is active
        Display->>Match: EM_GAME_MATCH_UPDATE
        Display->>Shooter: EM_GAME_SHOOTER_UPDATE
        Display->>Ball: EM_GAME_BALL_UPDATE
        Display->>Keeper: EM_GAME_KEEPER_UPDATE
    end

    Ball->>Goal: EM_GAME_GOAL_BALL_ARRIVED
    Keeper->>Goal: EM_GAME_GOAL_KEEPER_READY
    Goal->>Match: EM_GAME_MATCH_HIT_RESULT
    Match->>Buzzer: Result sound
    Match->>Display: EM_GAME_DISPLAY_UPDATE_MATCH
```

The display task validates every received snapshot before updating its local render model. Match accepts input only in the appropriate phase and accepts a result only while revealing the current kick. The shared tick stops before leaving gameplay for the RIP screen.

## Round and match completion

```mermaid
sequenceDiagram
    participant Tick as Display tick
    participant Match
    participant Objects as Goal/Ball/Keeper/Shooter
    participant Display
    participant EEPROM
    participant Buzzer

    Tick->>Match: EM_GAME_MATCH_UPDATE
    alt Round-end delay expires and rounds remain
        Match->>Match: Enter ROUND_START and run SETUP
        Match->>Objects: RESET signals
        Match->>Match: EM_GAME_MATCH_START_ROUND
        Match->>Display: Updated round snapshot
    else Round-end delay expires after result five
        Match->>Match: Evaluate winner
        opt New best score or equal score on harder difficulty
            Match->>EEPROM: save_score_record
        end
        Match->>Match: Remove gameplay tick from Display
        Match->>Display: EM_GAME_DISPLAY_SHOW_RIP
        alt Two-second timeout
            Match->>Match: EM_GAME_MATCH_RIP_TIMEOUT
        else Mode button
            Display->>Match: EM_GAME_MATCH_SKIP_RIP
        end
        Match->>Display: EM_GAME_DISPLAY_SHOW_GAME_OVER
        Match->>Buzzer: WIN or LOSE sound
    end
```

## Game-over actions

```mermaid
sequenceDiagram
    participant GameOver as Game-over screen
    participant Match
    participant Display

    alt Mode button: retry
        GameOver->>Match: EM_GAME_MATCH_RETRY
        Match->>Match: Reset scoreboard and start round one
        Match->>Display: EM_GAME_DISPLAY_SHOW_PENALTY
    else Up button: home
        GameOver->>Match: EM_GAME_MATCH_HOME
        Match->>Match: Reset scoreboard and enter MENU
        Match->>Display: EM_GAME_DISPLAY_UPDATE_MATCH
        Match->>Display: EM_GAME_DISPLAY_SHOW_MENU
    end
```
