#include "raylib.h"
#include "raymath.h"
#include "Enemy.h"
#include "Tower.h"      
#include "Projectile.h"  
#include <vector>

int main(void)
{
    // --- SETUP ---
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Siege of Gondor - Tower Defense");
    SetTargetFPS(60);

    // --- DEFINE PATH ---
    std::vector<Vector2>* path = new std::vector<Vector2>();
    path->push_back({ 0.0f, 360.0f });
    path->push_back({ 400.0f, 360.0f });
    path->push_back({ 400.0f, 150.0f });
    path->push_back({ 1100.0f, 150.0f });

    // --- LOAD ASSETS ---
    Texture2D texOrc = LoadTexture("assets/sprites/enemies/orc.png");
    Texture2D texUruk = LoadTexture("assets/sprites/enemies/uruk.png");
    Texture2D texTroll = LoadTexture("assets/sprites/enemies/troll.png");
    Texture2D texArcher = LoadTexture("assets/sprites/towers/gondor_soldier.png");
    Texture2D bg = LoadTexture("assets/sprites/environment/minastirith_bg.png");

    // --- GAME LISTS ---
    std::vector<Enemy> enemies;
    std::vector<Tower> towers;
    std::vector<Projectile> projectiles;

    // --- VARIABLES ---
    float spawnTimer = 0.0f;
    int gold = 400;

    // Wave Variables
    int waveNumber = 1;
    int enemiesPerWave = 5;
    int enemiesSpawned = 0;

    // EVOLVING MECHANIC & SELECTION
    int maxTowers = 5;
    TowerType selectedTower = TowerType::ARCHER;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // ---------------- UPDATE PHASE ----------------

        // 1. INPUT: Select Tower Type
        if (IsKeyPressed(KEY_ONE))   selectedTower = TowerType::ARCHER;
        if (IsKeyPressed(KEY_TWO))   selectedTower = TowerType::MELEE;
        if (IsKeyPressed(KEY_THREE)) selectedTower = TowerType::ICE;

        // 2. INPUT: Handle Clicks (Upgrade OR Place)
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            bool clickedExistingTower = false;

            // --- CHECK 1: Did we click an existing tower? (UPGRADE) ---
            for (Tower& t : towers) {
                if (t.IsClicked(mousePos)) {
                    clickedExistingTower = true;
                    int upgradeCost = t.GetUpgradeCost();

                    if (gold >= upgradeCost) {
                        gold -= upgradeCost;
                        t.Upgrade(); // Level up!
                    }
                    break; // Stop checking other towers
                }
            }

            // --- CHECK 2: If we didn't click a tower, place a new one! ---
            if (!clickedExistingTower) {
                // Determine Cost based on selection
                int cost = 0;
                if (selectedTower == TowerType::ARCHER) cost = 100;
                else if (selectedTower == TowerType::MELEE) cost = 75;
                else if (selectedTower == TowerType::ICE) cost = 150;

                // Place only if not on UI and have money/slots
                if (mousePos.y < screenHeight - 60) {
                    if (gold >= cost && towers.size() < maxTowers) {
                        towers.emplace_back(mousePos, texArcher, selectedTower);
                        gold -= cost;
                    }
                }
            }
        }

        // 3. WAVE LOGIC
        // Case A: Spawning
        if (enemiesSpawned < enemiesPerWave) {
            spawnTimer += dt;
            float spawnDelay = (waveNumber > 5) ? 0.8f : 1.5f;

            if (spawnTimer >= spawnDelay) {
                spawnTimer = 0.0f;
                // Determine Enemy Type
                EnemyType type = EnemyType::ORC;
                Texture2D tex = texOrc;

                if (waveNumber >= 3 && GetRandomValue(0, 10) > 5) {
                    type = EnemyType::URUK;
                    tex = texUruk;
                }
                if (waveNumber >= 5 && enemiesSpawned == enemiesPerWave - 1) {
                    type = EnemyType::TROLL;
                    tex = texTroll;
                }

                enemies.push_back(Enemy(type, path, tex));
                enemiesSpawned++;
            }
        }
        // Case B: Wave Cleared (Transition)
        else if (enemies.empty()) {
            spawnTimer += dt;
            if (spawnTimer > 2.0f) {
                waveNumber++;
                enemiesPerWave += 3;
                enemiesSpawned = 0;
                spawnTimer = 0.0f;
                gold += 100;

                // EVOLUTION: Unlock more tower slots!
                maxTowers += 2;
            }
        }

        // 4. Update Enemies
        for (int i = 0; i < enemies.size(); i++) {
            enemies[i].Update(dt);
            if (!enemies[i].IsAlive() || enemies[i].ReachedEnd()) {
                enemies.erase(enemies.begin() + i);
                i--;
            }
        }

        // 5. Update Towers
        for (Tower& t : towers) {
            t.Update(dt, enemies, projectiles);
        }

        // 6. Update Projectiles & Collisions
        for (int i = 0; i < projectiles.size(); i++) {
            projectiles[i].Update(dt);

            for (Enemy& e : enemies) {
                if (!e.IsAlive()) continue;

                if (CheckCollisionCircles(projectiles[i].position, 5, e.GetPosition(), e.GetRadius())) {
                    e.TakeDamage(projectiles[i].damage);

                    // --- ICE EFFECT ---
                    if (projectiles[i].type == ProjectileType::ICE) {
                        e.ApplySlow(0.5f, 2.0f); // 50% Speed for 2.0s
                    }

                    projectiles[i].active = false;
                    if (!e.IsAlive()) gold += 25;
                    break;
                }
            }

            if (!projectiles[i].active) {
                projectiles.erase(projectiles.begin() + i);
                i--;
            }
        }

        // ---------------- DRAW PHASE ----------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // 1. Draw Map
        Rectangle sourceRec = { 0.0f, 0.0f, (float)bg.width, (float)bg.height };
        Rectangle destRec = { 0.0f, 0.0f, (float)screenWidth, (float)screenHeight };
        DrawTexturePro(bg, sourceRec, destRec, { 0, 0 }, 0.0f, WHITE);

        // 2. Draw Path
        for (size_t i = 0; i < path->size() - 1; i++) {
            DrawLineEx((*path)[i], (*path)[i + 1], 30.0f, Fade(LIGHTGRAY, 0.5f));
        }

        // 3. Draw Entities
        for (const auto& t : towers) t.Draw();
        for (const auto& e : enemies) e.Draw();
        for (const auto& p : projectiles) p.Draw();

        // 4. DRAW HOVER UI (New!)
        // Show upgrade cost if mouse is over a tower
        for (const Tower& t : towers) {
            if (t.IsClicked(GetMousePosition())) {
                DrawText(TextFormat("Upgrade: %d g", t.GetUpgradeCost()), GetMouseX(), GetMouseY() - 30, 20, GREEN);
                DrawCircleLines(GetMouseX(), GetMouseY(), 20, GREEN); // Simple cursor highlight
            }
        }

        // 5. DRAW BOTTOM UI BAR
        DrawRectangle(0, screenHeight - 60, screenWidth, 60, Fade(BLACK, 0.8f));

        // Stats
        DrawText(TextFormat("Gold: %d", gold), 20, 20, 30, YELLOW);
        DrawText(TextFormat("Wave: %d", waveNumber), 20, 60, 30, WHITE);

        // Controls
        Color c1 = (selectedTower == TowerType::ARCHER) ? YELLOW : GRAY;
        Color c2 = (selectedTower == TowerType::MELEE) ? RED : GRAY;
        Color c3 = (selectedTower == TowerType::ICE) ? SKYBLUE : GRAY;

        DrawText(TextFormat("Towers: %d / %d", towers.size(), maxTowers), 20, screenHeight - 40, 20, WHITE);
        DrawText("[1] Archer (100g)", 200, screenHeight - 40, 20, c1);
        DrawText("[2] Melee (75g)", 400, screenHeight - 40, 20, c2);
        DrawText("[3] Ice (150g)", 600, screenHeight - 40, 20, c3);

        // Wave Status
        if (enemiesSpawned < enemiesPerWave) {
            DrawText("Incoming...", screenWidth - 150, screenHeight - 40, 20, ORANGE);
        }
        else if (enemies.empty()) {
            DrawText("Next Wave in 2s...", screenWidth - 200, screenHeight - 40, 20, GREEN);
        }
        else {
            DrawText("Defend!", screenWidth - 100, screenHeight - 40, 20, RED);
        }

        EndDrawing();
    }
    // --- CLEANUP ---
    UnloadTexture(texOrc);
    UnloadTexture(texUruk);
    UnloadTexture(texTroll);
    UnloadTexture(texArcher);
    UnloadTexture(bg);
    delete path;

    CloseWindow();
    return 0;
}