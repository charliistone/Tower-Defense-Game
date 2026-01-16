TOWER DEFENSE GAME - FRONTIERS
This project is a complete, playable Tower Defense game developed for the SENG479 Game Programming course. It is built from the ground up using C/C++ and the Raylib library, focusing on core engine concepts such as game loops, state management, and collision detection.



Game Concept & Mechanics
The game focuses on tactical positioning and resource management to defend a base against increasingly difficult waves of enemies.

Tactical Placement: Players must place towers in strategic locations to maximize area-of-effect and create efficient choke points.

Dynamic Wave Scaling: Features a sophisticated wave system where enemy density, health, and movement speed increase as the game progresses.

Economic Progression: Players manage a gold-based economy, earning currency from defeated enemies to invest in multi-tier tower upgrades.

Win/Loss Conditions: The game concludes if the base's health reaches zero or if the player successfully survives all scheduled waves.


Technical Implementation

Framework: Developed using Raylib with C/C++ for high-performance rendering and input handling.

Engine Performance: Optimized to maintain a consistent 60 FPS during normal gameplay, even during high-density enemy waves.

Modular Architecture: The project is organized across multiple source files (e.g., EnemySystem.cpp, TowerManager.cpp, GameState.cpp) to ensure clean logic and maintainability.

Collision Logic: Implements custom collision detection algorithms for projectile-to-enemy interactions and pathfinding constraints.


 Build & Execution Instructions

Environment: This project is provided as a Visual Studio Solution.

Setup:

Open the .sln file in Visual Studio.

Ensure the Raylib include and library paths are correctly mapped for the lab environment.

Compilation: Build the project using the "Release" configuration for optimal performance.

Binary: A pre-compiled executable is located in the /bin or /build folder.


Controls

Mouse Left-Click: Build towers and interact with the UI.

Mouse Right-Click: Cancel placement or interact with existing towers.

Spacebar: Start the next wave manually.

P / Esc: Pause the game and access the menu.


Credits & External Assets
All assets used in this project are royalty-free and sourced from the following providers:

Graphics: Sourced from [craftpix.net], [kenney.nl], [sorthen.itch.io]

Sound Effects: Sourced via [Freesound.org], [opengameart.org]

Fonts: Custom typography via [dafont.com]
