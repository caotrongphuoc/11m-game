<div align="center">

![Repo Traffic](https://komarev.com/ghpvc/?username=11m-game&label=Repo+Traffic&color=red&style=flat-square)

</div>

<p align="center">
  <img src="https://img.shields.io/badge/language-C%2B%2B-red?style=flat-square" alt="Language">
  <img src="https://img.shields.io/badge/mcu-STM32L151-red?style=flat-square" alt="MCU">
  <img src="https://img.shields.io/badge/framework-Active%20Kernel-red?style=flat-square" alt="Framework">
  <img src="https://img.shields.io/badge/hardware-AK%20Base%20Kit-red?style=flat-square" alt="Hardware">
</p>

# Eleven Meter - Game built on AK Embedded Base Kit

<hr>

## Documentation

| File | Description |
|---|---|
| [README.md](README.md) | Main project overview, hardware information, gameplay rules, and game-object descriptions. |
| [docs/01-guide-getting-started.md](docs/01-guide-getting-started.md) | Build, flash, play, and inspect the firmware. |
| [docs/02-guide-coding-rules.md](docs/02-guide-coding-rules.md) | Zomwar-style source conventions with current AK ownership rules. |
| [docs/03-design-sequence-object.md](docs/03-design-sequence-object.md) | State ownership and message sequences for gameplay objects. |
| [docs/04-design-sequence-runtime.md](docs/04-design-sequence-runtime.md) | End-to-end button, tick, task, result, and render signal flow. |
| [docs/05-ak-mcp-deployment.md](docs/05-ak-mcp-deployment.md) | Deploy and connect the AK documentation MCP server. |

## Introduction

Eleven Meter is a penalty shootout game built on top of the **AK Embedded Base Kit**. The project uses the AK (Active Kernel) event-driven framework to model the match, player input, animations, timers, and communication between gameplay objects.

While developing and playing Eleven Meter, the project demonstrates several embedded-system concepts:

- **Event-driven design:** Gameplay components react to signals and messages instead of blocking the processor.
- **Task coordination:** The match, shooter, goalkeeper, ball, goal, and display run as cooperating AK tasks.
- **Tick-driven gameplay:** One 20 ms display-owned timer posts update messages; each object advances its own timing without blocking.
- **Real-time interaction:** Buttons, timers, animations, and OLED rendering respond throughout each penalty round.

### I. Hardware

<table align="center">
  <tr>
    <td align="center"><img src="resources/images/board/ak-embedded-base-kit-version-3.jpg" alt="AK Embedded Base Kit - STM32L151 - v3.0" width="480"/></td>
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

**MCU naming convention:**

<div align="center">

| Part | Meaning |
|---|---|
| `STM32` | STMicroelectronics 32-bit MCU family. |
| `L` | Low-power series. |
| `151` | STM32L151 product line. |
| `C` | 48-pin package. |
| `B` | 128 KB Flash memory. |
| `T` | LQFP package. |
| `6` | Industrial temperature grade. |

</div>

<table align="center">
  <tr>
    <td align="center"><img src="resources/images/board/board-view-top-bottom.png" alt="AK Embedded Base Kit top and bottom views" width="900"/></td>
  </tr>
</table>
<p align="center"><strong><em>Figure 2:</em></strong> Board top and bottom views</p>

### II. Game Description and Objects

Eleven Meter recreates a five-round penalty shootout. The player selects a shooting direction before the countdown expires, while the goalkeeper uses difficulty-dependent logic to choose a dive direction. Each kick is evaluated as a goal, save, or miss, and the scoreboard determines the winner when the shootout is complete.

#### Objects in the Game:

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
Penalty screen ── shared tick ──> Match / Shooter / Ball / Keeper
Match ── accepted kick ──> Shooter
Shooter ── target ──> Ball ───────┐
Shooter ── shot zone ──> Keeper ──┤
                                  v
                                 Goal ── result ──> Match

Match / Shooter / Ball / Keeper ── snapshots ──> Display
```

> **Note:** See [Eleven Meter object sequences](docs/03-design-sequence-object.md) for detailed runtime message flows.

### III. How to Play:

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

> **Note:** See [Eleven Meter runtime flow](docs/04-design-sequence-runtime.md) for the detailed task and message flow.

## Project Structure

```text
11m-game/
├── application/                         # Eleven Meter application firmware
│   └── sources/app/
│       ├── game_eleven_meter/            # Message-driven gameplay objects
│       └── screens/                      # OLED screens and game rendering
├── boot/                                # AK bootloader source
├── docs/                                # Project guides and design documents
├── resources/
│   ├── bin/                             # Bootloader and Eleven Meter binaries
│   ├── board-assembly/                  # Board assembly drawing
│   ├── images/board/                    # Board and hardware images
│   ├── manufacturing/                   # Manufacturing assets
│   └── schematic/                       # Board schematics
└── AGENTS.md                            # Instructions for coding agents
```

## Build and Flash

The firmware expects the Arm GNU Toolchain 10.3 and STM32CubeProgrammer. Tool locations can be supplied without editing the Makefile:

```bash
make -C application \
  GCC_PATH=/path/to/gcc-arm-none-eabi-10.3-2021.10 \
  PROGRAMMER_PATH=/path/to/STM32CubeProgrammer/bin
```

The generated binary is `application/build_ak-base-kit-stm32l151-application/ak-base-kit-stm32l151-application.bin`.

Flash through the AK bootloader with `ak-flash` and the board UART port:

```bash
make -C application flash dev=/dev/ttyUSB0
```

Alternatively, omit `dev` to flash over ST-Link with STM32CubeProgrammer:

```bash
make -C application flash PROGRAMMER_PATH=/path/to/STM32CubeProgrammer/bin
```

Run `make -C application clean` before a full rebuild. The UART console uses 115200 baud, 8 data bits, no parity, and 1 stop bit.

## Development Rules

- Treat the [AK documentation MCP server](docs/05-ak-mcp-deployment.md) as the source of truth for kernel APIs and recipes.
- Keep handlers non-blocking; use timers and posted signals instead of delays or busy-waits.
- Keep mutable gameplay state inside its owning task implementation.
- Exchange gameplay commands, results, and render snapshots only through AK messages.
- Restrict feature work to `application/sources/app/` and `application/sources/driver/`; do not modify protected base-kit modules.
- Format changed C and C++ files with the root `.clang-format` before committing.

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

[GitHub](https://github.com/caotrongphuoc) · [LinkedIn](https://www.linkedin.com/in/cao-trong-phuoc/)
