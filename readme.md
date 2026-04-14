# SparCraft

SparCraft is a fast, deterministic combat simulator inspired by StarCraft unit battles.  
It is useful for AI search experiments, scripted policy evaluation, and reproducible combat testing.

## Quick Start

### Requirements

- CMake 3.24+
- C++23 compiler
- SFML 3 (for the GUI app)
- OpenGL (for the GUI app)

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
