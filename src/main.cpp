#include "raylib.h"
#include "raymath.h"
#include "Enemy.h"
#include "Tower.h"      
#include "Projectile.h"  
#include <vector>

// --- HELPER FUNCTIONS ---
float GetTowerRange(TowerType type) {
    switch (type) {
    case TowerType::ARCHER: return 200.0f;
    case TowerType::ICE: return 150.0f;
    case TowerType::MELEE: return 100.0f;
    default: return 0.0f;
    }
}
int GetTowerCost(TowerType type) {
    switch (type) {
    case TowerType::ARCHER: return 100;
    case TowerType::ICE: return 150;
    case TowerType::MELEE: return 75;
    default: return 0;
    }
}

// Check distance to ANY path (Top or Bottom)
float GetDistanceFromLine(Vector2 p, Vector2 a, Vector2 b) {
    Vector2 pa = Vector2Subtract(p, a);
    Vector2 ba = Vector2Subtract(b, a);
    float h = Clamp(Vector2DotProduct(pa, ba) / Vector2DotProduct(ba, ba), 0.0f, 1.0f);
    return Vector2Distance(p, Vector2Add(a, Vector2Scale(ba, h)));
}

// We now pass a LIST of paths to check
float GetMinDistanceToAnyPath(Vector2 pos, const std::vector<std::vector<Vector2>*>& allPaths) {
    float minDist = 9999.0f;
    for (auto* path : allPaths) {
        for (size_t i = 0; i < path->size() - 1; i++) {
            float dist = GetDistanceFromLine(pos, (*path)[i], (*path)[i + 1]);
            if (dist < minDist) minDist = dist;
        }
    }
    return minDist;
}

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Siege of Gondor - Pincer Strategy");
    SetTargetFPS(60);

    // --- DEFINE PATHS (STRATEGY MAP) ---
    // Path 1: Top Lane
    std::vector<Vector2>* pathTop = new std::vector<Vector2>();
    pathTop->push_back({ 0.0f, 200.0f });   // Start Top-Left
    pathTop->push_back({ 400.0f, 200.0f }); // Go Right
    pathTop->push_back({ 400.0f, 450.0f }); // Go Down to Middle
    pathTop->push_back({ 1050.0f, 450.0f }); // Go to Gate

    // Path 2: Bottom Lane
    std::vector<Vector2>* pathBottom = new std::vector<Vector2>();
    pathBottom->push_back({ 0.0f, 600.0f });   // Start Bottom-Left
    pathBottom->push_back({ 600.0f, 600.0f }); // Go Right
    pathBottom->push_back({ 600.0f, 450.0f }); // Go Up to Middle
    pathBottom->push_back({ 1050.0f, 450.0f }); // Go to Gate

    // Keep a list of all paths for collision checking
    std::vector<std::vector<Vector2>*> allPaths;
    allPaths.push_back(pathTop);
    allPaths.push_back(pathBottom);

    // --- LOAD ASSETS ---
    Texture2D texOrc = LoadTexture("assets/sprites/enemies/orc.png");
    Texture2D texUruk = LoadTexture("assets/sprites/enemies/uruk.png");
    Texture2D texTroll = LoadTexture("assets/sprites/enemies/troll.png");
    Texture2D texArcher = LoadTexture("assets/sprites/towers/gondor_soldier.png");
    Texture2D bg = LoadTexture("assets/sprites/environment/minastirith_bg.png");
    Texture2D texRoad = LoadTexture("assets/sprites/environment/road_texture.png");
    Texture2D texCity = LoadTexture("assets/sprites/environment/minastirith_city.png");

    std::vector<Enemy> enemies;
    std::vector<Tower> towers;
    std::vector<Projectile> projectiles;

    float spawnTimer = 0.0f;
    int gold = 500; // More starting gold for 2 lanes!
    int waveNumber = 1;
    int enemiesPerWave = 8; // More enemies!
    int enemiesSpawned = 0;
    int maxTowers = 6;
    TowerType selectedTower = TowerType::ARCHER;
    const int gridSize = 64;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        Vector2 mousePos = GetMousePosition();

        // ---------------- UPDATE PHASE ----------------
        if (IsKeyPressed(KEY_ONE))   selectedTower = TowerType::ARCHER;
        if (IsKeyPressed(KEY_TWO))   selectedTower = TowerType::MELEE;
        if (IsKeyPressed(KEY_THREE)) selectedTower = TowerType::ICE;

        // GRID & PLACEMENT
        int gridX = (int)(mousePos.x / gridSize) * gridSize;
        int gridY = (int)(mousePos.y / gridSize) * gridSize;
        Vector2 snapPos = { (float)gridX + 32, (float)gridY + 32 };

        bool isValidPlacement = true;
        const char* invalidReason = "";

        if (mousePos.y > screenHeight - 60) isValidPlacement = false;
        else {
            if (gold < GetTowerCost(selectedTower)) { isValidPlacement = false; invalidReason = "Too Expensive!"; }
            else if (towers.size() >= maxTowers) { isValidPlacement = false; invalidReason = "Max Towers!"; }

            for (const Tower& t : towers) {
                if (CheckCollisionPointCircle(snapPos, t.GetPosition(), 10.0f)) { isValidPlacement = false; invalidReason = "Occupied!"; }
            }

            // CHECK DISTANCE TO BOTH PATHS
            float distToRoad = GetMinDistanceToAnyPath(snapPos, allPaths);

            // Road is now thinner (50px), so safety distance is smaller (50px)
            if (isValidPlacement && distToRoad < 50.0f) {
                isValidPlacement = false;
                invalidReason = "Blocked!";
            }
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            bool clickedExisting = false;
            for (Tower& t : towers) {
                if (t.IsClicked(mousePos)) {
                    clickedExisting = true;
                    if (gold >= t.GetUpgradeCost()) { gold -= t.GetUpgradeCost(); t.Upgrade(); }
                    break;
                }
            }
            if (!clickedExisting && isValidPlacement) {
                towers.emplace_back(snapPos, texArcher, selectedTower);
                gold -= GetTowerCost(selectedTower);
            }
        }

        // SPAWNING
        if (enemiesSpawned < enemiesPerWave) {
            spawnTimer += dt;
            float delay = (waveNumber > 5) ? 0.6f : 1.2f; // Faster spawns
            if (spawnTimer >= delay) {
                spawnTimer = 0.0f;
                EnemyType type = EnemyType::ORC;
                Texture2D tex = texOrc;
                if (waveNumber >= 3 && GetRandomValue(0, 10) > 6) { type = EnemyType::URUK; tex = texUruk; }
                if (waveNumber >= 5 && enemiesSpawned == enemiesPerWave - 1) { type = EnemyType::TROLL; tex = texTroll; }

                // RANDOM PATH SELECTION!
                // 50% chance to go Top, 50% Bottom
                std::vector<Vector2>* chosenPath = (GetRandomValue(0, 1) == 0) ? pathTop : pathBottom;

                enemies.push_back(Enemy(type, chosenPath, tex));
                enemiesSpawned++;
            }
        }
        else if (enemies.empty()) {
            spawnTimer += dt;
            if (spawnTimer > 2.0f) {
                waveNumber++; enemiesPerWave += 4; enemiesSpawned = 0; spawnTimer = 0.0f; gold += 150; maxTowers += 2;
            }
        }

        // UPDATE ENTITIES
        for (int i = 0; i < enemies.size(); i++) {
            enemies[i].Update(dt);
            if (!enemies[i].IsAlive() || enemies[i].ReachedEnd()) { enemies.erase(enemies.begin() + i); i--; }
        }
        for (Tower& t : towers) t.Update(dt, enemies, projectiles);
        for (int i = 0; i < projectiles.size(); i++) {
            projectiles[i].Update(dt);
            for (Enemy& e : enemies) {
                if (e.IsAlive() && CheckCollisionCircles(projectiles[i].position, 5, e.GetPosition(), e.GetRadius())) {
                    e.TakeDamage(projectiles[i].damage);
                    if (projectiles[i].type == ProjectileType::ICE) e.ApplySlow(0.5f, 2.0f);
                    projectiles[i].active = false;
                    if (!e.IsAlive()) gold += 25;
                    break;
                }
            }
            if (!projectiles[i].active) { projectiles.erase(projectiles.begin() + i); i--; }
        }

        // ---------------- DRAW PHASE ----------------
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexturePro(bg, { 0,0,(float)bg.width,(float)bg.height }, { 0,0,(float)screenWidth,(float)screenHeight }, { 0,0 }, 0, WHITE);

        // DRAW BOTH PATHS
        for (auto* path : allPaths) {
            for (size_t i = 0; i < path->size() - 1; i++) {
                Vector2 start = (*path)[i];
                Vector2 end = (*path)[i + 1];
                float dist = Vector2Distance(start, end);
                Vector2 dir = Vector2Normalize(Vector2Subtract(end, start));

                // Thinner Road: 50px Width
                for (float d = 0; d < dist; d += 25.0f) {
                    Vector2 pos = Vector2Add(start, Vector2Scale(dir, d));
                    DrawTexturePro(texRoad, { 0,0,(float)texRoad.width,(float)texRoad.height },
                        { pos.x, pos.y, 50.0f, 50.0f }, { 25.0f, 25.0f }, 0.0f, WHITE);
                }
            }
        }

        // DRAW CITY (Scaled & Positioned)
        float cityScale = 1.3f;
        Vector2 gatePos = (*pathTop)[pathTop->size() - 1]; // End of road
        Vector2 cityOrigin = {
            (float)texCity.width * cityScale * 0.38f,
            (float)texCity.height * cityScale * 0.92f
        };

        DrawTexturePro(texCity,
            { 0, 0, (float)texCity.width, (float)texCity.height },
            { gatePos.x, gatePos.y, (float)texCity.width * cityScale, (float)texCity.height * cityScale },
            cityOrigin,
            0.0f, WHITE);

        // ENTITIES
        for (const auto& t : towers) t.Draw();
        for (const auto& e : enemies) e.Draw();
        for (const auto& p : projectiles) p.Draw();

        // UI & GHOST
        bool hover = false;
        for (const Tower& t : towers) {
            if (t.IsClicked(mousePos)) {
                hover = true;
                DrawText(TextFormat("UPGRADE: %dg", t.GetUpgradeCost()), mousePos.x, mousePos.y - 40, 20, GREEN);
                DrawCircleLines(t.GetPosition().x, t.GetPosition().y, t.GetRange(), GREEN);
            }
        }
        if (!hover && mousePos.y < screenHeight - 60) {
            float range = GetTowerRange(selectedTower);
            Color c = isValidPlacement ? Fade(GREEN, 0.5f) : Fade(RED, 0.5f);
            DrawCircleV(snapPos, range, Fade(c, 0.3f));
            DrawCircleLines(snapPos.x, snapPos.y, range, c);
            if (!isValidPlacement) DrawText(invalidReason, snapPos.x, snapPos.y - 20, 20, RED);
        }

        // BOTTOM BAR
        DrawRectangle(0, screenHeight - 60, screenWidth, 60, Fade(BLACK, 0.8f));
        DrawText(TextFormat("Gold: %d  |  Wave: %d", gold, waveNumber), 20, 20, 20, YELLOW);
        DrawText("[1] Archer  [2] Melee  [3] Ice", 400, screenHeight - 40, 20, WHITE);

        EndDrawing();
    }

    UnloadTexture(texOrc); UnloadTexture(texUruk); UnloadTexture(texTroll); UnloadTexture(texArcher);
    UnloadTexture(bg); UnloadTexture(texRoad); UnloadTexture(texCity);
    delete pathTop; delete pathBottom;
    CloseWindow();
    return 0;
}