# Siege of Gondor

## Project Overview
This project is a complete, playable Tower Defense game developed for the SENG479 Game Programming course. It is built from the ground up using C/C++ and the Raylib library, focusing on core game programming concepts including game loops, rendering, input handling, and game state management.

## Game Concept and Mechanics
The game emphasizes tactical positioning and resource management to defend a base against progressively difficult enemy waves.

* **Tactical Placement:** Players must position towers in strategic locations to maximize area-of-effect and create efficient choke points.
* **Dynamic Wave Scaling:** A sophisticated wave system increases enemy density, health, and movement speed as the game progresses.
* **Economic Progression:** Players manage a gold-based economy, earning currency from defeated enemies to invest in multi-tier tower upgrades.
* **Win/Loss Conditions:** The game concludes if the base's health reaches zero or if the player successfully survives all scheduled waves.

## Technical Implementation
* **Framework:** Developed using Raylib with C/C++.
* **Engine Performance:** The codebase is optimized to maintain a minimum of 60 frames per second (FPS) during normal gameplay.
* **Modular Architecture:** The project is organized across multiple source files to ensure clean logic and maintainability.
* **Functions and Logic:** All functions utilize descriptive naming conventions, and complex logic is supported by explanatory comments.
* **Collision Logic:** Implements custom collision detection algorithms for projectile-to-enemy interactions and pathfinding constraints.

## Build and Execution Instructions
This project is submitted as a Visual Studio Solution.

### Setup
1. Open the `.sln` file in Visual Studio.
2. Verify that the Raylib include and library paths are correctly mapped for the laboratory environment.

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
All external assets are royalty-free and sourced from the following providers:

* **Graphics:**
    * [CraftPix](https://craftpix.net)
    * [Kenney](https://kenney.nl)
    * [Sorthen (Itch.io)](https://sorthen.itch.io)
* **Sound Effects:**
    * [Freesound](https://freesound.org)
    * [OpenGameArt](https://opengameart.org)
* **Fonts:**
    * [DaFont](https://www.dafont.com)
 
# 🏰 Siege of Gondor

A strategy-based **Tower Defense Game** developed in **C**, inspired by the legendary world of *The Lord of the Rings*.

### 📜 The Story
As fans of LOTR series, we designed a story that stays true to the epic defense of Gondor. As the Lord of Gondor, you must protect the main tower from the relentless assault of Sauron's forces.

---

### 🎮 Gameplay & Objective
* **Goal:** Defend the main tower against **20 waves** of enemies.
* **Victory:** Survive all 20 waves.
* **Defeat:** If the main tower falls before the 20th wave, it's Game Over.
* **Economy:** Earn coins by slaying enemies to build or upgrade your defenses.

### 🏹 Defense Towers
You can build and upgrade three types of towers using the numeric keys:

| Key | Tower Type | Description |
| :---: | :--- | :--- |
| `1` | **ARCHER** | High attack speed, moderate damage. |
| `2` | **MELEE** | Heavy damage at close range. |
| `3` | **ICE** | Deals damage and **slows down** enemies. |

> [!TIP]
> Click on an existing tower to **upgrade** it if you have enough coins!

### 👹 The Enemy Horde
The forces of darkness come in three ranks, each with unique health and speed stats:
* **Orc:** Fast but weak.
* **Uruk:** Balanced strength and speed.
* **Troll:** Slow moving but massive health pools.

### ✨ Special Abilities
Inspired by iconic movie scenes, you have two legendary powers at your disposal:

* **Gandalf (Key: `Q`)**: *"You Shall Not Pass!"* – Instantly freezes all enemies in their current position.
* **Rohirrim (Key: `W`)**: Call for the cavalry! – A devastating charge that wipes out enemies on the path, just like the films.

---

## Project Team & Contact Info

This project was developed by the following team members. Click on numbers or email addresses to contact.

| Name Surname | GitHub | Email |
| :--- | :---: | :--- |
| Ahmet Can Cengiz | [🔗 Profil](https://github.com/charliistone) | [📧 s220201023@ankarabilim.edu.tr](mailto:s220201023@ankarabilim.edu.tr) |
| Ahmet Said Kuruoğlu | [🔗 Profil](https://github.com/kullaniciadi) | [📧 s220204008@ankarabilim.edu.tr](mailto:s220204008@ankarabilim.edu.tr) |
| Anday Turgut| [🔗 Profil](https://github.com/andayk) | [📧 s220204050@ankarabilim.edu.tr](mailto:s220204050@ankarabilim.edu.tr) |
| Bartu Turgut | [🔗 Profil](https://github.com/bartu33) | [📧 s220201032@ankarabilim.edu.tr](mailto:s220201032@ankarabilim.edu.tr) |
| Salih Aydos | [🔗 Profil](https://github.com/LordChancelor) | [📧 s220204002@ankarabilim.edu.tr](mailto:s220204002@ankarabilim.edu.tr) |
| Mustafa Kayalıca | [🔗 Profil](https://github.com/kullaniciadi) | [📧 s220201014@ankarabilim.edu.tr](mailto:s220201014@ankarabilim.edu.tr) |
