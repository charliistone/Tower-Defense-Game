# Siege of Gondor

## Project Overview
**Siege of Gondor** is a strategy-based Tower Defense game developed for the **SENG479 Game Programming** course. Built natively using **C/C++** and the **Raylib** library, the project demonstrates core game engineering concepts including custom game loops, rendering pipelines, input handling, and state management.

Inspired by the lore of *The Lord of the Rings*, the game places the player in command of Gondor's defenses. The objective is to protect the main tower from the relentless assault of Sauron's forces through tactical resource management and strategic positioning.

## Game Mechanics & Design
The gameplay focuses on a tactical defense loop where players must withstand **20 progressively difficult waves** of enemies.

### Core Systems
* **Tactical Placement:** Players must utilize the terrain to create choke points and maximize the area-of-effect for their defensive structures.
* **Economic Progression:** A gold-based economy rewards players for eliminating enemies, allowing for the purchase of new towers or the upgrading of existing ones.
* **Win/Loss Conditions:** The game concludes with a victory if all 20 waves are repelled. A "Game Over" state is triggered if the main tower's health is depleted.

### Defense Towers
Players can deploy and upgrade three distinct tower classes using the numeric keys:

| Key | Type | Characteristics |
| :--- | :--- | :--- |
| **1** | **Archer Tower** | High attack speed with moderate damage output. Ideal for single-target DPS. |
| **2** | **Melee Tower** | Heavy damage at close range. Best utilized at choke points. |
| **3** | **Ice Tower** | Applies a movement speed debuff (slow) to enemies while dealing damage. |

### Enemy Ranks
The enemy waves consist of distinct unit types, ranging from fodder to boss-tier threats:

* **Orc:** High movement speed, low health. Serves as the primary swarm unit.
* **Uruk:** Balanced attributes in terms of speed and durability.
* **Commander:** Elite infantry with superior health and damage resistance compared to standard units.
* **Troll:** Slow movement speed with a massive health pool (Tank class). Requires sustained damage to defeat.
* **Witch King:** The ultimate boss unit. Features extreme durability and serves as the primary threat in final waves.

### Special Abilities
Players have access to high-impact abilities for emergency crowd control:
* **Gandalf (Q):** Temporarily freezes all active enemies in their current position.
* **Rohirrim (W):** Initiates a cavalry charge that clears enemies along the path.

## Technical Implementation
This project eschews commercial game engines in favor of a code-first approach to demonstrate low-level game programming skills.

* **Framework:** Developed in C/C++ utilizing the Raylib library for window management and graphics.
* **Optimization:** The game loop is optimized to maintain a stable **60 FPS** performance target.
* **Architecture:** The codebase features a modular architecture with separated logic for collision detection, entity management, and UI rendering to ensure maintainability.
* **Collision Logic:** Implements custom algorithms for projectile-to-enemy interaction and pathfinding constraints.

## Build and Execution
The project is configured as a Visual Studio Solution.

### Setup & Compilation
1. Open the solution file (`.sln`) in **Visual Studio**.
2. Ensure Raylib include/library paths are correctly linked for the build environment.
3. Set the configuration to **Release**.
4. Build the solution (`Ctrl+Shift+B`).
5. The executable will be generated in the `build` or `bin` directory.

### Controls
* **Mouse Left-Click:** Build towers.
* **1 / 2 / 3:** Select Tower Type (Archer / Melee / Ice).
* **Q:** Activate Ability: Gandalf.
* **W:** Activate Ability: Rohirrim.
* **Esc:** Quit the game.

## Development Team
**University:** Ankara Bilim University
**Course:** SENG479 Game Programming

| Name Surname | GitHub Profile | Contact |
| :--- | :--- | :--- |
| Ahmet Can Cengiz | [charliistone](https://github.com/charliistone) | s220201023@ankarabilim.edu.tr |
| Ahmet Said Kuruoğlu | [Merchrior](https://github.com/Merchrior) | s220204008@ankarabilim.edu.tr |
| Anday Turgut | [andayk](https://github.com/andayk) | s220204050@ankarabilim.edu.tr |
| Bartu Turgut | [bartu33](https://github.com/bartu33) | s220201032@ankarabilim.edu.tr |
| Salih Aydos | [LordChancelor](https://github.com/LordChancelor) | s220204002@ankarabilim.edu.tr |
| Mustafa Kayalıca | [Kayalicaa](https://github.com/Kayalicaa) | s220201014@ankarabilim.edu.tr |

## Credits & Assets
All external assets used in this project are sourced from the following providers:

* **Audio & Sound Effects:**
    * [Freesound.org](https://freesound.org)
    * [MyInstants.com](https://www.myinstants.com)
    * [Voicy.network](https://voicy.network)

* **Visual Assets:**
    * **Characters:** [OpenGameArt.org](https://opengameart.org), [Itch.io](https://itch.io), Microsoft Bing
    * **Towers & Structures:** [Kenney.nl](https://kenney.nl), Microsoft Bing
    * **End Game Screens:** [OpenArt.ai](https://openart.ai)

* **Fonts:**
    * [DaFont](https://www.dafont.com)
