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
    Match->>Display: EM_GAME_DISPLAY_UPDATE_MATCH
```

The display task validates every received snapshot before updating its local render model. Match accepts input only in the appropriate phase and accepts a result only while revealing the current kick. The shared tick stops before leaving gameplay for the RIP screen.
