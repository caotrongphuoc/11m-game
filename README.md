<div align="center">

![Repo Traffic](https://komarev.com/ghpvc/?username=11m-game&label=Repo+Traffic&color=blue&style=flat-square)

</div>

# Eleven Meter - Game built on AK Embedded Base Kit

<!-- Add the Eleven Meter banner or gameplay GIF here when available.
<p align="center">
  <img src="resources/images/screens/banner_game_eleven_meter.gif" alt="Eleven Meter" width="960"/>
</p>
-->

<hr>

## Gameplay Demo

> Gameplay video will be added when the first public demo is available.

## Documentation

| File | Description |
|---|---|
| [README.md](README.md) | Main project overview, hardware information, gameplay rules, and game-object descriptions. |
| [docs/02-guide-coding-rules.md](docs/02-guide-coding-rules.md) | Coding conventions used by the project. |
| [docs/03-design-sequence-object.md](docs/03-design-sequence-object.md) | State ownership and runtime message sequences for the gameplay objects. |

## Introduction

Eleven Meter is a penalty shootout game built on top of the **AK Embedded Base Kit**. The project uses the AK (Active Kernel) event-driven framework to model the match, player input, animations, timers, and communication between gameplay objects.

While developing and playing Eleven Meter, the project demonstrates several embedded-system concepts:

- **Event-driven design:** Gameplay components react to signals and messages instead of blocking the processor.
- **Task coordination:** The match, shooter, goalkeeper, ball, goal, and display run as cooperating AK tasks.
- **State management:** Match progression is represented by explicit states from the menu to game over.
- **Real-time interaction:** Buttons, timers, animations, and OLED rendering respond throughout each penalty round.

### I. Hardware

<table align="center">
  <tr>
    <td align="center"><img src="hardware/images/ak-embedded-base-kit-version-3.jpg" alt="AK Embedded Base Kit - STM32L151 - v3.0" width="480"/></td>
  </tr>
</table>
<p align="center"><strong><em>Figure 1:</em></strong> AK Embedded Base Kit - STM32L151</p>

The [AK Embedded Base Kit](https://epcb.vn/products/ak-embedded-base-kit-lap-trinh-nhung-vi-dieu-khien-mcu) is an evaluation platform for learning intermediate and advanced embedded software development.

The kit integrates a **1.54-inch OLED display**, **three push buttons**, and **a buzzer**, providing the controls and feedback required for a compact event-driven game. It also includes RS485, Qwiic, and Grove interfaces for further embedded experiments.

**MCU overview:**

```text
SoC Name : STM32L151CBT6
RAM      : 16 KB

Flash Partitions Layout
-----------------------
[ 0x08000000 - 0x08001FFF ] : Bootloader Partition (8 KB)
=> AK Bootloader

[ 0x08002000 - 0x08002FFF ] : BSF Shared Partition (4 KB)
=> Data shared between Bootloader and Application

[ 0x08003000 - 0x0801FFFF ] : Application Partition (116 KB)
=> Eleven Meter firmware
```

<table align="center">
  <tr>
    <td align="center"><img src="hardware/images/board-view-top-bottom.png" alt="AK Embedded Base Kit top and bottom views" width="900"/></td>
  </tr>
</table>
<p align="center"><strong><em>Figure 2:</em></strong> Board top and bottom views</p>

### II. Game Description and Objects

Eleven Meter recreates a five-round penalty shootout. The player selects a shooting direction before the countdown expires, while the goalkeeper uses difficulty-dependent logic to choose a dive direction. Each kick is evaluated as a goal, save, or miss, and the scoreboard determines the winner when the shootout is complete.

<!-- Add a menu or gameplay screenshot here when available.
<p align="center">
  <img src="resources/images/screens/scr_game_penalty.png" alt="Eleven Meter gameplay" width="600"/>
</p>
-->

#### Objects in the Game

| Object | Description |
|---|---|
| **Match** | Coordinates the menu, round setup, kick selection, result reveal, round completion, and game-over flow. |
| **Shooter** | Represents the player and plays the shooting animation after a direction is selected. |
| **Ball** | Moves from the penalty spot toward the selected left, center, or right target. A shot may also travel wide. |
| **Keeper** | Selects and animates a dive direction according to the current difficulty. |
| **Goal** | Resolves the ball and goalkeeper directions into a goal, save, or miss result. |
| **Scoreboard** | Tracks rounds, goals, saves, misses, and the final winner across five kicks. |

Each gameplay task owns its mutable state inside its implementation file. Objects do not read another task's state through shared globals; they exchange commands, animation snapshots, and results through AK messages.

```text
Match
  └── kick request ──> Shooter
                         ├── ball target ──> Ball ───────┐
                         └── shot zone ────> Keeper ─────┤
                                                        v
                                                       Goal
                                                        │
                                              goal/save/miss result
                                                        │
                                                        v
                                                      Match

Match / Shooter / Ball / Keeper ── view messages ──> Display
```

### III. How to Play

#### Main Menu

- Press **[Up]** or **[Down]** to cycle through **Easy**, **Normal**, and **Hard** difficulty.
- Press **[Mode]** to start the shootout.

#### Penalty Kick

- Press **[Up]** to kick left.
- Press **[Mode]** to kick toward the center.
- Press **[Down]** to kick right.
- Choose a direction before the three-second countdown expires.

#### Game Over

- Press **[Mode]** to retry with the selected difficulty.
- Press **[Up]** to return to the main menu.

#### Game Mechanics

- **Shootout length:** A match contains five penalty kicks.
- **Results:** Each attempt is recorded as a goal, save, or miss.
- **Difficulty:** Easy, Normal, and Hard modes change the goalkeeper's ability to match the selected direction.
- **Reaction pressure:** Very fast input increases the chance of the shot missing the goal.
- **Score tracking:** The current round and result totals are shown on the OLED display.
- **Match flow:** A short result phase separates each kick; the final result proceeds through the RIP and Game Over screens.

### IV. Basic Game Sequence Logic

```text
Main Menu
    |
    v
Select Difficulty
    |
    v
Start Match -> Round Setup -> Three-Second Selection
                                  |
                       +----------+----------+
                       |          |          |
                      Left      Center      Right
                       +----------+----------+
                                  |
                                  v
                              Shooter
                             /       \
                            v         v
                          Ball      Keeper
                            \         /
                             v       v
                        Goal Evaluation
                                  |
                    +-------------+-------------+
                    |                           |
               Next Round                  Match Complete
                                                |
                                                v
                                        RIP -> Game Over
                                                |
                                         Retry or Home
```

> See [Eleven Meter object sequences](docs/03-design-sequence-object.md) for the detailed task and message flow.

## Project Structure

```text
11m-game/
├── application/                         # Eleven Meter application firmware
│   └── sources/app/
│       ├── game/game_eleven_meter/       # Message-driven gameplay objects
│       └── screens/                      # OLED screens and game rendering
├── boot/                                # AK bootloader source
├── docs/                                # Project guides and design documents
├── hardware/                            # Board images, schematic, and binaries
└── AGENTS.md                            # Instructions for coding agents
```

## References

| Topic | Link |
|---|---|
| AK Embedded Base Kit | <https://github.com/the-ak-foundation/ak-base-kit-stm32l151> |
| AK Foundation | <https://github.com/the-ak-foundation> |
| Embedded Training Program | <https://github.com/the-ak-foundation/embedded-training-program> |
| Hardware Vendor | <https://epcb.vn/products/ak-embedded-base-kit-lap-trinh-nhung-vi-dieu-khien-mcu> |

## Contact & Support

**Cao Trong Phuoc** - Software Engineer - Embedded Systems

Thank you for visiting this repository. If you have questions, suggestions, or feedback about Eleven Meter or embedded firmware development, feel free to get in touch.

<a href="https://github.com/caotrongphuoc">
  <img src="https://img.shields.io/badge/GitHub-caotrongphuoc-181717?style=for-the-badge&logo=github&logoColor=white" alt="GitHub"/>
</a>

<a href="https://www.linkedin.com/in/cao-trong-phuoc/">
  <img src="https://img.shields.io/badge/LinkedIn-Cao%20Trong%20Phuoc-0A66C2?style=for-the-badge&logo=linkedin&logoColor=white" alt="LinkedIn"/>
</a>
