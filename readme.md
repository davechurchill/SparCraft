# SparCraft

SparCraft is a cross-platform open source abstract StarCraft combat simulation package written in C++. It can be used to create standalone combat simulations or be imported into an existing BWAPI-based StarCraft bot to provide additional AI functionality.

SparCraft was built with speed in mind, and uses a system of frame fast-forwarding to bypass game frames during which no combat decisions are being made (ie: during attack cooldowns, movements, etc). Using this system, SparCraft can easily perform tens of millions of unit actions per second.

SparCraft is abstract in the sense that it does not perfectly model all aspects of StarCraft. Since no StarCraft source code is available, all combat logic is our best guess at what is happening in the game engine. It is meant to be a simple testbed for AI systems playing an abstract version of a very complex RTS game. It does not need to perfectly model all StarCraft features to do this job.

SparCraft currently contains the following functionality:
  * Accurately models all unit size, weapon, armor and damage types
  * Accurately models all unit upgrades and research types
  * Accurately models unit speeds, [attack cooldowns and animation frame timings](https://docs.google.com/spreadsheet/ccc?key=0An3xUNEy2rixdGtleHI4Y2FqUU1wNUthY05leVY2X1E#gid=0)
  * Can extract map files straight from StarCraft for use in simulation

SparCraft Unit Movements
  * While SparCraft supports arbitrary pixel-precision movements, `GameState::generateMoves()` only generates UP, DOWN, LEFT, RIGHT movements of a fixed length. This abstraction is to give the search algorithms a reasonable number of actions to search over. This means the standalone version of SparCraft only supports 4-directional movement, but if you edit the code yourself you can implement arbitrary movements.

SparCraft does not yet contain the following functionality:
  * *Unit Collisions* - This is by design. Unit collisions are expensive, the point of the system is to be fast
  * Fog of War
  * Spell-Casting or Splash Damage (Muta 1-hit only)
  * No Burrowing or Cloaking / Detection
  * Non-Deterministic Aspects (such as random hit chance)
  * Vision Up Ramps
  * Unit turning or acceleration

Despite these features not yet being included, SparCraft simulations demonstrate a high level of accuracy with what happens in an actual StarCraft battle. The accuracy of these simulations however diminishes as battle sizes increase, due to the lack of unit collision functionality.

## StarCraft AI Research

One of the main reasons for the release of this project was so that there could be a system for testing various AI algorithms for RTS combat. Many publications have presented work on RTS combat AI, however each one is implemented in a different system, and are often limited by simulation speed or by the time it takes to write a full simulation system.

Not only is SparCraft a full simulation system, but it also includes several AI algorithms we have developed for RTS combat.

### Scripted Players

Many static scripted players come with SparCraft, they are:

    AttackClosest
    AttackDPS
    AttackWeakest
    Cluster
    Kiter
    KiterDPS
    NOKDPS
    Random

### Alpha Beta

We have developed a version of Alpha Beta tree search called Alpha Beta Considering Durations (ABCD). This algorithm was introduced in the following 2012 AIIDE paper: [Fast Heuristic Search for RTS Game Combat Scenarios](https://davechurchill.ca/publications/pdf/aiide12-combat.pdf).

The algorithm is implemented in `src/AlphaBetaSearch.cpp`

### UCT Tree Search

UCT is another popular tree search algorithm, and we have modified it for use with RTS combat. It is available in `src/UCTSearch.cpp`

A paper detailing this UCT implementation is currently submitted to CIG2013 and will be posted here if accepted.

### Portfolio Greedy Search

Our newest greedy search technique which loses to AB/UCT at small combat sizes, but easily defeats them at larger sizes. It is implemented in `src/PortfolioGreedySearch.cpp`

You can read the details about these methods in our 2013 CIG paper: [Portfolio Greedy Search and Simulation for Large-Scale Combat in Starcraft](https://davechurchill.ca/publications/pdf/combat13.pdf)

## Quick Start

### Requirements

- CMake 3.24+
- C++23 compiler
- SFML 3 (for the GUI app)
- OpenGL (for the GUI app)

### Windows (Open Included Visual Studio Solution)

1. Set `SFML_DIR` (used by `vs/SparCraft_main.vcxproj`):

```bat
set SFML_DIR=C:\path\to\SFML-3.0.1
```

2. Open `vs\SparCraft.sln` in Visual Studio.
3. Select `x64` and `Release` (or `Debug`).
4. Set startup project to `SparCraft_main`.
5. Build and run from Visual Studio (`F5`).

### Windows (CMake + MSBuild)

1. Build the app:

```bat
build_win.bat
```

2. Run:

```bat
bin\SparCraft.exe ExperimentFilename.txt
```

### Windows (Visual Studio Generator Helper)

If you use `generate_vs.bat`, set `SFML_DIR` first so CMake can find SFML:

```bat
set SFML_DIR=C:\path\to\SFML-3.0.1
generate_vs.bat
```

### Linux

1. Configure and build:

```bash
cmake -S . -B build -DSPARCRAFT_BUILD_APP=ON -DSPARCRAFT_BUILD_BWAPIDATA=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --target SparCraft --parallel 8
```

2. Run:

```bash
./bin/SparCraft
```

## Tests

Build and run tests:

```bash
cmake -S . -B build -DSPARCRAFT_BUILD_TESTS=ON -DSPARCRAFT_BUILD_APP=OFF
cmake --build build --config Debug --target SparCraftTests
ctest --test-dir build -C Debug --output-on-failure
```

## Project Layout

- `src/` core simulation, search, players, and GUI sources
- `bin/` runtime output binaries and sample experiment files
- `build/` generated build files
- `vs/` included Visual Studio solution and project files
