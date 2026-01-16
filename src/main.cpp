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
    // Ensure these files exist in your assets folder!
    Texture2D texOrc = LoadTexture("assets/sprites/enemies/orc.png");
    Texture2D texUruk = LoadTexture("assets/sprites/enemies/uruk.png"); // Optional
    Texture2D texTroll = LoadTexture("assets/sprites/enemies/troll.png"); // Optional
    Texture2D texArcher = LoadTexture("assets/sprites/towers/gondor_soldier.png"); // <--- NEW: Tower Texture
    Texture2D bg = LoadTexture("assets/sprites/environment/minastirith_bg.png");

    // --- GAME LISTS ---
    std::vector<Enemy> enemies;
    std::vector<Tower> towers;           // <--- NEW: List of towers
    std::vector<Projectile> projectiles; // <--- NEW: List of bullets

    // --- VARIABLES ---
    float spawnTimer = 0.0f;
    int gold = 300; // Starting money
    int waveNumber = 1;       // Start at Wave 1
    int enemiesPerWave = 5;   // 5 enemies in the first wave
    int enemiesSpawned = 0;   // How many have we spawned so far?

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // ---------------- UPDATE PHASE ----------------

        // 1. Spawn Enemies (Simple Wave)
        if (enemiesSpawned < enemiesPerWave) {
            spawnTimer += dt;

            // Make spawning faster in later waves (Harder!)
            float spawnDelay = (waveNumber > 5) ? 0.8f : 1.5f;

            if (spawnTimer >= spawnDelay) {
                spawnTimer = 0.0f;

                // --- DETERMINE ENEMY TYPE ---
                EnemyType type = EnemyType::ORC;
                Texture2D tex = texOrc;

                // Wave 3+: 50% chance for Uruk
                if (waveNumber >= 3) {
                    if (GetRandomValue(0, 10) > 5) {
                        type = EnemyType::URUK;
                        tex = texUruk;
                    }
                }

                // Wave 5+: The last enemy is always a TROLL
                if (waveNumber >= 5 && enemiesSpawned == enemiesPerWave - 1) {
                    type = EnemyType::TROLL;
                    tex = texTroll;
                }

                enemies.push_back(Enemy(type, path, tex));
                enemiesSpawned++;
            }
        }
        // Case B: Wave is finished (Spawned everyone AND everyone is dead)
        else if (enemies.empty()) {
            // WAIT PHASE: Give the player 2 seconds to breathe before next wave
            spawnTimer += dt;
            if (spawnTimer > 2.0f) {
                waveNumber++;           // Increase Wave
                enemiesPerWave += 3;    // Add 3 more enemies next time
                enemiesSpawned = 0;     // Reset counter
                spawnTimer = 0.0f;      // Reset timer
                gold += 100;            // Give bonus gold
            }
        }

        // 2. Input: Place Tower (Left Click)
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();
            if (gold >= 100) {
                // Add a new tower at mouse position
                towers.emplace_back(mousePos, texArcher);
                gold -= 100;
            }
        }

        // 3. Update Enemies
        for (int i = 0; i < enemies.size(); i++) {
            enemies[i].Update(dt);
            if (!enemies[i].IsAlive() || enemies[i].ReachedEnd()) {
                enemies.erase(enemies.begin() + i);
                i--;
            }
        }

        // 4. Update Towers (They need to know about enemies and projectiles)
        for (Tower& t : towers) {
            t.Update(dt, enemies, projectiles);
        }

        // 5. Update Projectiles & Check Collisions
        for (int i = 0; i < projectiles.size(); i++) {
            projectiles[i].Update(dt);

            // Check collision with ANY enemy
            for (Enemy& e : enemies) {
                if (!e.IsAlive()) continue;

                // Simple Circle Collision Check
                // Projectile Radius = 4, Enemy Radius = e.GetRadius()
                if (CheckCollisionCircles(projectiles[i].position, 4, e.GetPosition(), e.GetRadius())) {
                    e.TakeDamage(projectiles[i].damage);
                    projectiles[i].active = false; // Destroy bullet
                    break; // Bullet hit one enemy, stop checking others
                }
            }

            // Remove dead bullets
            if (!projectiles[i].active) {
                projectiles.erase(projectiles.begin() + i);
                i--;
            }
        }

        // ---------------- DRAW PHASE ----------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // 1. Draw Background (Stretched)
        // Note: We declare these variables ONCE inside this block
        Rectangle sourceRec = { 0.0f, 0.0f, (float)bg.width, (float)bg.height };
        Rectangle destRec = { 0.0f, 0.0f, (float)screenWidth, (float)screenHeight };
        DrawTexturePro(bg, sourceRec, destRec, { 0, 0 }, 0.0f, WHITE);

        // 2. Draw Path Line
        for (size_t i = 0; i < path->size() - 1; i++) {
            DrawLineEx((*path)[i], (*path)[i + 1], 30.0f, Fade(LIGHTGRAY, 0.5f));
        }

        // 3. Draw Everything Else
        for (const auto& t : towers) t.Draw();
        for (const auto& e : enemies) e.Draw();
        for (const auto& p : projectiles) p.Draw();

        // 4. Draw UI
        DrawText(TextFormat("Gold: %d", gold), 20, 20, 30, YELLOW);
        DrawText(TextFormat("Wave: %d", waveNumber), 20, 100, 30, WHITE);

        if (enemiesSpawned < enemiesPerWave) {
            // Case 1: Still spawning enemies
            DrawText(TextFormat("Enemies Incoming: %d/%d", enemiesSpawned, enemiesPerWave), 20, 130, 20, ORANGE);
        }
        else if (!enemies.empty()) {
            // Case 2: Spawning finished, but enemies are still alive (YOUR CURRENT STATE)
            DrawText(TextFormat("Kill the remaining %d enemies!", enemies.size()), 20, 130, 20, RED);
        }
        else {
            // Case 3: Everyone dead, waiting for next wave logic to trigger
            DrawText("Wave Complete! Next wave starting...", 20, 130, 20, GREEN);
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