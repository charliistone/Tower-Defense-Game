#include "raylib.h"
#include "raymath.h"
#include "Enemy.h"
#include "Tower.h"      
#include "Projectile.h"  
#include <vector>
#include <string>
#include "../IntroScreen.h"

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

float GetDistanceFromLine(Vector2 p, Vector2 a, Vector2 b) {
    Vector2 pa = Vector2Subtract(p, a);
    Vector2 ba = Vector2Subtract(b, a);
    float h = Clamp(Vector2DotProduct(pa, ba) / Vector2DotProduct(ba, ba), 0.0f, 1.0f);
    return Vector2Distance(p, Vector2Add(a, Vector2Scale(ba, h)));
}

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

const int MAX_BLOOD = 100;
const int COST_GANDALF = 40;
const int COST_ROHIRRIM = 60;

// --- ROHIRRIM STRUCT ---
struct Rohirrim {
    Vector2 position;
    std::vector<Vector2>* path;
    int currentPoint;
    bool active;

    // Animasyon Kareleri
    const std::vector<Texture2D>* frames;

    float animTimer;
    int currentFrameIndex;

    Rohirrim(std::vector<Vector2>* p, const std::vector<Texture2D>* animFrames) {
        path = p;
        frames = animFrames;

        currentPoint = (int)path->size() - 1;
        position = (*path)[currentPoint];
        active = true;

        animTimer = 0.0f;
        currentFrameIndex = 0;
    }

    void Update(float dt) {
        if (!active) return;
        float speed = 350.0f;

        // 1. Animasyon
        animTimer += dt;
        if (animTimer >= 0.08f) {
            animTimer = 0.0f;
            currentFrameIndex++;
            if (currentFrameIndex >= frames->size()) currentFrameIndex = 0;
        }

        // 2. Hareket
        if (currentPoint > 0) {
            Vector2 target = (*path)[currentPoint - 1];
            Vector2 dir = Vector2Normalize(Vector2Subtract(target, position));
            position = Vector2Add(position, Vector2Scale(dir, speed * dt));

            if (Vector2Distance(position, target) < 15.0f) currentPoint--;
        }
        else {
            active = false;
        }
    }

    void Draw() {
        if (!active || frames->empty()) return;

        Texture2D currentTex = (*frames)[currentFrameIndex];
        Rectangle source = { 0, 0, -(float)currentTex.width, (float)currentTex.height };
        Rectangle dest = { position.x, position.y, 80, 80 };
        Vector2 origin = { 40, 40 };

        DrawTexturePro(currentTex, source, dest, origin, 0.0f, WHITE);
    }
};

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Siege of Gondor - Return of the King");
    SetTargetFPS(60);

    IntroScreen intro;
    intro.Init();
    while (!WindowShouldClose() && !intro.IsFinished())
    {
        intro.Update();
        BeginDrawing();
        ClearBackground(BLACK);
        intro.Draw();
        EndDrawing();
    }
    intro.Unload();

    // --- PATHS ---
    std::vector<Vector2>* pathTop = new std::vector<Vector2>();
    pathTop->push_back({ 0.0f, 200.0f });
    pathTop->push_back({ 400.0f, 200.0f });
    pathTop->push_back({ 400.0f, 450.0f });
    pathTop->push_back({ 1050.0f, 450.0f });

    std::vector<Vector2>* pathBottom = new std::vector<Vector2>();
    pathBottom->push_back({ 0.0f, 600.0f });
    pathBottom->push_back({ 600.0f, 600.0f });
    pathBottom->push_back({ 600.0f, 450.0f });
    pathBottom->push_back({ 1050.0f, 450.0f });

    std::vector<std::vector<Vector2>*> allPaths;
    allPaths.push_back(pathTop);
    allPaths.push_back(pathBottom);

    // --- ASSETS ---
    Texture2D texOrc = LoadTexture("assets/sprites/enemies/orc.png");
    Texture2D texUruk = LoadTexture("assets/sprites/enemies/uruk.png");
    Texture2D texTroll = LoadTexture("assets/sprites/enemies/troll.png");
    Texture2D bg = LoadTexture("assets/sprites/environment/minastirith_bg.png");
    Texture2D texRoad = LoadTexture("assets/sprites/environment/road_texture.png");
    Texture2D texCity = LoadTexture("assets/sprites/environment/minastirith_city.png");
    Texture2D texGandalf = LoadTexture("assets/sprites/gandalf.png");

    // --- YENÝ KULELERÝ YÜKLE ---
    Texture2D texTowerArcher = LoadTexture("assets/sprites/towers/tower_archer.png");
    Texture2D texTowerMelee = LoadTexture("assets/sprites/towers/tower_melee.png");
    Texture2D texTowerIce = LoadTexture("assets/sprites/towers/tower_ice.png");

    // --- ROHIRRIM FRAMES ---
    std::vector<Texture2D> rohirrimFrames;
    rohirrimFrames.push_back(LoadTexture("assets/sprites/Knight_gallop1.png"));
    rohirrimFrames.push_back(LoadTexture("assets/sprites/Knight_gallop2.png"));
    rohirrimFrames.push_back(LoadTexture("assets/sprites/Knight_gallop3.png"));
    rohirrimFrames.push_back(LoadTexture("assets/sprites/Knight_gallop4.png"));
    rohirrimFrames.push_back(LoadTexture("assets/sprites/Knight_gallop5.png"));

    std::vector<Enemy> enemies;
    std::vector<Tower> towers;
    std::vector<Projectile> projectiles;
    std::vector<Rohirrim> riders;

    float spawnTimer = 0.0f;
    int gold = 450;
    int waveNumber = 1;
    int enemiesPerWave = 8;
    int enemiesSpawned = 0;
    int maxTowers = 6;
    TowerType selectedTower = TowerType::ARCHER;
    const int gridSize = 64;

    int urukBlood = 0;
    float flashTimer = 0.0f;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        Vector2 mousePos = GetMousePosition();

        // --- SKILL INPUTS ---
        if (IsKeyPressed(KEY_Q)) {
            if (urukBlood >= COST_GANDALF) {
                urukBlood -= COST_GANDALF;
                flashTimer = 1.0f;
                for (Enemy& e : enemies) e.ApplyStun(3.0f);
            }
        }
        if (IsKeyPressed(KEY_W)) {
            if (urukBlood >= COST_ROHIRRIM) {
                urukBlood -= COST_ROHIRRIM;
                riders.emplace_back(pathTop, &rohirrimFrames);
                riders.emplace_back(pathBottom, &rohirrimFrames);
                riders.emplace_back(pathTop, &rohirrimFrames);
                riders.emplace_back(pathBottom, &rohirrimFrames);
            }
        }

        // --- GAME LOGIC ---
        if (IsKeyPressed(KEY_ONE))   selectedTower = TowerType::ARCHER;
        if (IsKeyPressed(KEY_TWO))   selectedTower = TowerType::MELEE;
        if (IsKeyPressed(KEY_THREE)) selectedTower = TowerType::ICE;

        int gridX = (int)(mousePos.x / gridSize) * gridSize;
        int gridY = (int)(mousePos.y / gridSize) * gridSize;
        Vector2 snapPos = { (float)gridX + 32, (float)gridY + 32 };
        bool isValidPlacement = true;
        const char* invalidReason = "";

        if (mousePos.y > screenHeight - 60) isValidPlacement = false;
        else {
            float range = GetTowerRange(selectedTower);
            float distToRoad = GetMinDistanceToAnyPath(snapPos, allPaths);

            if (gold < GetTowerCost(selectedTower)) { isValidPlacement = false; invalidReason = "Too Expensive!"; }
            else if (towers.size() >= maxTowers) { isValidPlacement = false; invalidReason = "Max Towers!"; }
            for (const Tower& t : towers) {
                if (CheckCollisionPointCircle(snapPos, t.GetPosition(), 10.0f)) { isValidPlacement = false; invalidReason = "Occupied!"; }
            }
            if (isValidPlacement && distToRoad < 50.0f) { isValidPlacement = false; invalidReason = "Blocked!"; }
            if (isValidPlacement && distToRoad > range) { isValidPlacement = false; invalidReason = "Too Far!"; }
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
                // --- KULE TÝPÝNE GÖRE RESÝM SEÇ ---
                Texture2D textureToUse = texTowerArcher;
                if (selectedTower == TowerType::MELEE) textureToUse = texTowerMelee;
                else if (selectedTower == TowerType::ICE) textureToUse = texTowerIce;

                towers.emplace_back(snapPos, textureToUse, selectedTower);
                gold -= GetTowerCost(selectedTower);
            }
        }

        // SPAWNING
        if (enemiesSpawned < enemiesPerWave) {
            spawnTimer += dt;
            float delay = (waveNumber > 5) ? 0.6f : 1.2f;
            if (spawnTimer >= delay) {
                spawnTimer = 0.0f;
                EnemyType type = EnemyType::ORC;
                Texture2D tex = texOrc;
                if (waveNumber >= 3 && GetRandomValue(0, 10) > 6) { type = EnemyType::URUK; tex = texUruk; }
                if (waveNumber >= 5 && enemiesSpawned == enemiesPerWave - 1) { type = EnemyType::TROLL; tex = texTroll; }

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

        // UPDATES
        for (int i = 0; i < enemies.size(); i++) {
            enemies[i].Update(dt);
            if (!enemies[i].IsAlive()) {
                urukBlood += enemies[i].GetManaReward();
                if (urukBlood > MAX_BLOOD) urukBlood = MAX_BLOOD;
                gold += 15;
                enemies.erase(enemies.begin() + i); i--; continue;
            }
            if (enemies[i].ReachedEnd()) { enemies.erase(enemies.begin() + i); i--; }
        }

        for (int i = 0; i < riders.size(); i++) {
            riders[i].Update(dt);
            for (Enemy& e : enemies) {
                if (e.IsAlive() && CheckCollisionCircles(riders[i].position, 30, e.GetPosition(), 20)) {
                    e.TakeDamage(100);
                    if (!e.IsAlive()) {
                        urukBlood += e.GetManaReward();
                        if (urukBlood > MAX_BLOOD) urukBlood = MAX_BLOOD;
                    }
                }
            }
            if (!riders[i].active) { riders.erase(riders.begin() + i); i--; }
        }

        for (Tower& t : towers) t.Update(dt, enemies, projectiles);

        for (int i = 0; i < projectiles.size(); i++) {
            projectiles[i].Update(dt);
            for (Enemy& e : enemies) {
                if (e.IsAlive() && CheckCollisionCircles(projectiles[i].position, 5, e.GetPosition(), e.GetRadius())) {
                    e.TakeDamage(projectiles[i].damage);
                    if (projectiles[i].type == ProjectileType::ICE) e.ApplySlow(0.5f, 2.0f);
                    projectiles[i].active = false;
                    if (!e.IsAlive()) {
                        urukBlood += e.GetManaReward();
                        if (urukBlood > MAX_BLOOD) urukBlood = MAX_BLOOD;
                        gold += 15;
                    }
                    break;
                }
            }
            if (!projectiles[i].active) { projectiles.erase(projectiles.begin() + i); i--; }
        }

        // --- DRAWING ---
        BeginDrawing();

        Camera2D cam = { 0 };
        cam.zoom = 1.0f;
        BeginMode2D(cam);
        ClearBackground(RAYWHITE);
        DrawTexturePro(bg, { 0,0,(float)bg.width,(float)bg.height }, { 0,0,(float)screenWidth,(float)screenHeight }, { 0,0 }, 0, WHITE);

        for (auto* path : allPaths) {
            for (size_t i = 0; i < path->size() - 1; i++) {
                Vector2 start = (*path)[i];
                Vector2 end = (*path)[i + 1];
                float dist = Vector2Distance(start, end);
                Vector2 dir = Vector2Normalize(Vector2Subtract(end, start));
                for (float d = 0; d < dist; d += 25.0f) {
                    Vector2 pos = Vector2Add(start, Vector2Scale(dir, d));
                    DrawTexturePro(texRoad, { 0,0,(float)texRoad.width,(float)texRoad.height },
                        { pos.x, pos.y, 50.0f, 50.0f }, { 25.0f, 25.0f }, 0.0f, WHITE);
                }
            }
        }

        float cityScale = 1.3f;
        Vector2 gatePos = (*pathTop)[pathTop->size() - 1];
        Vector2 cityOrigin = { (float)texCity.width * cityScale * 0.38f, (float)texCity.height * cityScale * 0.92f };
        DrawTexturePro(texCity, { 0,0,(float)texCity.width,(float)texCity.height },
            { gatePos.x, gatePos.y, (float)texCity.width * cityScale, (float)texCity.height * cityScale },
            cityOrigin, 0.0f, WHITE);

        for (const auto& t : towers) t.Draw();
        for (const auto& e : enemies) e.Draw();
        for (auto& r : riders) r.Draw();
        for (const auto& p : projectiles) p.Draw();

        // --- GANDALF ANIMASYON ---
        if (flashTimer > 0.0f) {
            flashTimer -= dt;
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(WHITE, flashTimer * 0.4f));

            float cellSize = 64.0f;
            int animFrame = (int)((1.0f - flashTimer) * 8.0f) % 7;
            int row = 2;

            Rectangle source = { animFrame * cellSize, row * cellSize, cellSize, cellSize };
            DrawTexturePro(texGandalf, source,
                { (float)screenWidth / 2 - 64, (float)screenHeight / 2 - 80, 128, 128 },
                { 0,0 }, 0.0f, Fade(WHITE, flashTimer + 0.5f));
        }

        bool hover = false;
        for (const Tower& t : towers) {
            if (t.IsClicked(mousePos)) {
                hover = true;
                DrawText(TextFormat("UPGRADE: %dg", t.GetUpgradeCost()), mousePos.x, mousePos.y - 40, 20, GREEN);
                DrawCircleLines((int)t.GetPosition().x, (int)t.GetPosition().y, t.GetRange(), GREEN);
            }
        }
        if (!hover && mousePos.y < screenHeight - 60) {
            float range = GetTowerRange(selectedTower);
            Color c = isValidPlacement ? Fade(GREEN, 0.5f) : Fade(RED, 0.5f);
            DrawCircleV(snapPos, range, Fade(c, 0.3f));
            DrawCircleLines((int)snapPos.x, (int)snapPos.y, range, c);
            if (!isValidPlacement) DrawText(invalidReason, (int)snapPos.x, (int)snapPos.y - 20, 20, RED);
        }

        EndMode2D();

        DrawRectangle(0, screenHeight - 60, screenWidth, 60, Fade(BLACK, 0.9f));
        DrawText(TextFormat("Gold: %d", gold), 20, screenHeight - 45, 20, YELLOW);
        DrawText(TextFormat("Wave: %d", waveNumber), 20, screenHeight - 25, 20, WHITE);

        Color c1 = (selectedTower == TowerType::ARCHER) ? YELLOW : GRAY;
        Color c2 = (selectedTower == TowerType::MELEE) ? RED : GRAY;
        Color c3 = (selectedTower == TowerType::ICE) ? SKYBLUE : GRAY;
        int midX = screenWidth / 2 - 150;
        DrawText("[1] Archer", midX, screenHeight - 40, 20, c1);
        DrawText("[2] Melee", midX + 110, screenHeight - 40, 20, c2);
        DrawText("[3] Ice", midX + 220, screenHeight - 40, 20, c3);

        int rightX = screenWidth - 450;
        int barY = screenHeight - 35;

        DrawRectangleLines(rightX, barY, 120, 15, GRAY);
        float bloodPct = (float)urukBlood / MAX_BLOOD;
        DrawRectangle(rightX + 1, barY + 1, (int)(118 * bloodPct), 13, MAROON);
        DrawText("BLOOD", rightX + 40, barY + 3, 10, WHITE);

        Color cQ = (urukBlood >= COST_GANDALF) ? WHITE : DARKGRAY;
        Color cW = (urukBlood >= COST_ROHIRRIM) ? WHITE : DARKGRAY;

        DrawText("[Q] GANDALF", rightX + 130, barY - 5, 20, cQ);
        DrawText("[W] ROHIRRIM", rightX + 280, barY - 5, 20, cW);

        EndDrawing();
    }

    UnloadTexture(texOrc); UnloadTexture(texUruk); UnloadTexture(texTroll);

    // --- YENÝ TEXTURELARI BOÞALT ---
    UnloadTexture(texTowerArcher);
    UnloadTexture(texTowerMelee);
    UnloadTexture(texTowerIce);

    UnloadTexture(bg); UnloadTexture(texRoad); UnloadTexture(texCity);
    UnloadTexture(texGandalf);

    for (auto& tex : rohirrimFrames) UnloadTexture(tex);

    delete pathTop; delete pathBottom;
    CloseWindow();
    return 0;
}