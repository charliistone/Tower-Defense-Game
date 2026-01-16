# TOWER DEFENSE: Frontiers

## Project Overview
[cite_start]This project is a complete, playable Tower Defense game developed for the SENG479 Game Programming course[cite: 2, 7]. [cite_start]It is built from the ground up using C/C++ and the Raylib library [cite: 7, 12][cite_start], focusing on core game programming concepts including game loops, rendering, input handling, and game state management[cite: 8].

## Game Concept and Mechanics
The game emphasizes tactical positioning and resource management to defend a base against progressively difficult enemy waves.

* **Tactical Placement:** Players must position towers in strategic locations to maximize area-of-effect and create efficient choke points.
* **Dynamic Wave Scaling:** A sophisticated wave system increases enemy density, health, and movement speed as the game progresses.
* **Economic Progression:** Players manage a gold-based economy, earning currency from defeated enemies to invest in multi-tier tower upgrades.
* **Win/Loss Conditions:** The game concludes if the base's health reaches zero or if the player successfully survives all scheduled waves.

## Technical Implementation
* [cite_start]**Framework:** Developed using Raylib with C/C++[cite: 12].
* [cite_start]**Engine Performance:** The codebase is optimized to maintain a minimum of 60 frames per second (FPS) during normal gameplay.
* [cite_start]**Modular Architecture:** The project is organized across multiple source files to ensure clean logic and maintainability[cite: 16].
* [cite_start]**Functions and Logic:** All functions utilize descriptive naming conventions, and complex logic is supported by explanatory comments[cite: 17].
* [cite_start]**Collision Logic:** Implements custom collision detection algorithms for projectile-to-enemy interactions and pathfinding constraints[cite: 8].

## Build and Execution Instructions
[cite_start]This project is submitted as a Visual Studio Solution[cite: 14, 29].

### Setup
1. [cite_start]Open the `.sln` file in Visual Studio[cite: 14].
2. [cite_start]Verify that the Raylib include and library paths are correctly mapped for the laboratory environment[cite: 28].

### Compilation
1. Set the solution configuration to **Release** for optimal performance.
2. Build the solution (Ctrl+Shift+B).

### Binary
A compiled executable is included in the `build` or `bin` folder.

## Controls
* **Mouse Left-Click:** Build towers and interact with the User Interface.
* **Mouse Right-Click:** Cancel current placement or interact with existing towers.
* **Spacebar:** Manually trigger the next wave.
* **P / Esc:** Pause the gameplay and access the main menu.

## Credits and External Assets
[cite_start]All external assets are royalty-free and sourced from the following providers[cite: 49, 53]:

* **Graphics:**
    * [cite_start][CraftPix](https://craftpix.net) [cite: 50]
    * [cite_start][Kenney](https://kenney.nl) [cite: 50]
    * [cite_start][Sorthen (Itch.io)](https://sorthen.itch.io) [cite: 50]
* **Sound Effects:**
    * [cite_start][Freesound](https://freesound.org) [cite: 51]
    * [cite_start][OpenGameArt](https://opengameart.org) [cite: 51]
* **Fonts:**
    * [cite_start][DaFont](https://www.dafont.com) [cite: 52]

---
[cite_start]*Developed for SENG479 Game Programming - 2025-2026 Fall Semester* [cite: 1]
