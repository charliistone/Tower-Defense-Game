#include "raylib.h"
#include "raymath.h"
#include "Enemy.h"
#include "Tower.h"      
#include "Projectile.h"  
#include <vector>
#include <string>

// --- OYUN DURUMLARI ---
enum class GameScreen { TITLE, LEVEL_SELECT, GAMEPLAY, VICTORY, GAMEOVER };

// --- SABİT AYARLAR ---
const int TILE_SIZE = 64;
const int MAP_ROWS = 12; // Yükseklik genelde sabit kalır (Ekran yüksekliği)
// NOT: MAP_COLS artık sabit değil, her level için değişebilir!

struct EnemyWave {
    int enemyCount;
    EnemyType enemyType;
    float spawnInterval;
    float speedMultiplier;
    int healthBonus;
};

// --- LEVEL YAPISI (GÜNCELLENDİ) ---
struct LevelData {
    int levelID;
    const char* name;
    Texture2D background;
    Color bgColor;

    // ARTIK SABİT DEĞİL, VEKTÖR KULLANIYORUZ
    std::vector<std::vector<int>> tileMap;

    std::vector<std::vector<Vector2>*> paths;
    int startGold;
    int mapWidth; // Haritanın piksel genişliği (cols * TILE_SIZE)
    int cols;     // Bu level kaç sütun? (30, 50, 100...)

    Vector2 castlePos;
    float castleScale;
    std::vector<EnemyWave> waves;
};

// --- DİNAMİK YOL BULUCU (GÜNCELLENDİ) ---
// Artık sabit MAP_COLS yerine map[0].size() kullanarak genişliği anlıyor.
std::vector<std::vector<Vector2>*> GeneratePathsFromMap(const std::vector<std::vector<int>>& map) {
    std::vector<std::vector<Vector2>*> allPaths;

    int rows = map.size();
    if (rows == 0) return allPaths;
    int cols = map[0].size(); // Haritanın genişliğini dinamik al

    for (int startY = 0; startY < rows; startY++) {
        for (int startX = 0; startX < cols; startX++) {
            if (map[startY][startX] == 2) {
                std::vector<Vector2>* newPath = new std::vector<Vector2>();
                int currX = startX, currY = startY;
                int prevX = -1, prevY = -1;
                bool reachedEnd = false;

                while (!reachedEnd) {
                    newPath->push_back({ (float)currX * TILE_SIZE + TILE_SIZE / 2, (float)currY * TILE_SIZE + TILE_SIZE / 2 });
                    if (map[currY][currX] == 3) { reachedEnd = true; break; }

                    int nextX = -1, nextY = -1;
                    int dx[] = { 1, 0, -1, 0 }; int dy[] = { 0, 1, 0, -1 };

                    for (int i = 0; i < 4; i++) {
                        int cx = currX + dx[i], cy = currY + dy[i];
                        // Sınır kontrolünde dinamik 'cols' kullanıyoruz
                        if (cx >= 0 && cx < cols && cy >= 0 && cy < rows) {
                            if ((cx != prevX || cy != prevY) && (map[cy][cx] == 1 || map[cy][cx] == 3)) {
                                nextX = cx; nextY = cy; break;
                            }
                        }
                    }
                    if (nextX != -1) { prevX = currX; prevY = currY; currX = nextX; currY = nextY; }
                    else break;
                }
                if (newPath->size() > 1) allPaths.push_back(newPath); else delete newPath;
            }
        }
    }
    return allPaths;
}

// --- UI & YARDIMCI FONKSİYONLAR ---
bool GuiButton(Rectangle rect, const char* text, Texture2D texNormal, Texture2D texHover) {
    Vector2 mousePos = GetMousePosition();
    bool hover = CheckCollisionPointRec(mousePos, rect);
    bool clicked = false;
    if (hover) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            if (texHover.id > 0) DrawTexturePro(texHover, { 0,0,(float)texHover.width,(float)texHover.height }, { rect.x + 2, rect.y + 2, rect.width - 4, rect.height - 4 }, { 0,0 }, 0, WHITE);
            else { DrawRectangleRec(rect, Color{ 80, 80, 80, 255 }); DrawRectangleLinesEx(rect, 4, DARKBROWN); }
        }
        else {
            if (texHover.id > 0) DrawTexturePro(texHover, { 0,0,(float)texHover.width,(float)texHover.height }, rect, { 0,0 }, 0, WHITE);
            else { DrawRectangleRec(rect, Color{ 169, 169, 169, 255 }); DrawRectangleLinesEx(rect, 4, GOLD); }
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) clicked = true;
        }
    }
    else {
        if (texNormal.id > 0) DrawTexturePro(texNormal, { 0,0,(float)texNormal.width,(float)texNormal.height }, rect, { 0,0 }, 0, WHITE);
        else { DrawRectangleRec(rect, DARKGRAY); DrawRectangleLinesEx(rect, 4, BLACK); }
    }
    int fontSize = 20; int textWidth = MeasureText(text, fontSize);
    int textX = (int)(rect.x + (rect.width - textWidth) / 2); int textY = (int)(rect.y + (rect.height - fontSize) / 2);
    DrawText(text, textX + 2, textY + 2, fontSize, BLACK);
    DrawText(text, textX, textY, fontSize, hover ? DARKBLUE : RAYWHITE);
    return clicked;
}

float GetTowerRange(TowerType type) { if (type == TowerType::ARCHER) return 250.0f; if (type == TowerType::ICE) return 180.0f; return 100.0f; }
int GetTowerCost(TowerType type) { if (type == TowerType::ARCHER) return 100; if (type == TowerType::ICE) return 150; return 75; }
float GetMinDistanceToAnyPath(Vector2 pos, const std::vector<std::vector<Vector2>*>& allPaths) {
    float minDist = 9999.0f;
    for (auto* path : allPaths) for (size_t i = 0; i < path->size() - 1; i++) if (CheckCollisionPointCircle(pos, (*path)[i], 30)) minDist = 0;
    return minDist;
}

// Efekt Sistemleri
struct BloodParticle { Vector2 position; int currentFrame; float animTimer; bool active; };
class BloodManager {
public:
    std::vector<BloodParticle> particles; Texture2D texture; int framesPerRow;
    BloodManager() { framesPerRow = 4; }
    void Init(Texture2D tex, int frames) { texture = tex; framesPerRow = frames; }
    void Spawn(Vector2 pos) { particles.push_back({ pos, 0, 0.0f, true }); }
    void Update(float dt) {
        for (int i = 0; i < particles.size(); i++) {
            if (!particles[i].active) continue;
            particles[i].animTimer += dt;
            if (particles[i].animTimer >= 0.08f) {
                particles[i].animTimer = 0.0f; particles[i].currentFrame++;
                if (particles[i].currentFrame >= framesPerRow) { particles[i].active = false; particles.erase(particles.begin() + i); i--; }
            }
        }
    }
    void Draw() {
        if (texture.id <= 0) return;
        float frameWidth = (float)texture.width / framesPerRow;
        for (const auto& p : particles) {
            Rectangle source = { p.currentFrame * frameWidth, 0, frameWidth, (float)texture.height };
            Rectangle dest = { p.position.x, p.position.y, frameWidth, (float)texture.height };
            Vector2 origin = { frameWidth / 2, (float)texture.height / 2 };
            DrawTexturePro(texture, source, dest, origin, 0.0f, WHITE);
        }
    }
};

struct Rohirrim {
    Vector2 position; std::vector<Vector2>* path; int currentPoint; bool active;
    const std::vector<Texture2D>* frames; float animTimer; int currentFrameIndex;
    Rohirrim(std::vector<Vector2>* p, const std::vector<Texture2D>* animFrames) {
        path = p; frames = animFrames; currentPoint = (int)path->size() - 1; position = (*path)[currentPoint]; active = true; animTimer = 0.0f; currentFrameIndex = 0;
    }
    void Update(float dt) {
        if (!active) return;
        float speed = 350.0f; animTimer += dt;
        if (animTimer >= 0.08f) { animTimer = 0.0f; currentFrameIndex++; if (currentFrameIndex >= frames->size()) currentFrameIndex = 0; }
        if (currentPoint > 0) {
            Vector2 target = (*path)[currentPoint - 1];
            Vector2 dir = Vector2Normalize(Vector2Subtract(target, position));
            position = Vector2Add(position, Vector2Scale(dir, speed * dt));
            if (Vector2Distance(position, target) < 15.0f) currentPoint--;
        }
        else { active = false; }
    }
    void Draw() {
        if (!active || frames->empty()) return;
        Texture2D currentTex = (*frames)[currentFrameIndex];
        Rectangle source = { 0, 0, (float)currentTex.width, (float)currentTex.height };
        Rectangle dest = { position.x, position.y, 80, 80 }; Vector2 origin = { 40, 40 };
        DrawTexturePro(currentTex, source, dest, origin, 0.0f, WHITE);
    }
};

const int MAX_BLOOD = 100;
const int COST_GANDALF = 40;
const int COST_ROHIRRIM = 60;

// --- MAIN ---
int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Siege of Gondor - Variable Map Sizes");
    SetTargetFPS(60);

    // Assets
    Texture2D texMenuBg = LoadTexture("assets/ui/menu_bg.png");
    Texture2D texBtnNormal = LoadTexture("assets/ui/btn_default.png");
    Texture2D texBtnHover = LoadTexture("assets/ui/btn_hover.png");
    Texture2D texOrc = LoadTexture("assets/sprites/enemies/orc.png");
    Texture2D texUruk = LoadTexture("assets/sprites/enemies/uruk.png");
    Texture2D texTroll = LoadTexture("assets/sprites/enemies/troll.png");
    Texture2D texRoad = LoadTexture("assets/sprites/environment/road_texture.png");
    Texture2D texCity = LoadTexture("assets/sprites/environment/minastirith_city.png");
    Texture2D texTowerArcher = LoadTexture("assets/sprites/towers/tower_archer.png");
    Texture2D texTowerMelee = LoadTexture("assets/sprites/towers/tower_melee.png");
    Texture2D texTowerIce = LoadTexture("assets/sprites/towers/tower_ice.png");
    Texture2D texProjectiles = LoadTexture("assets/sprites/projectiles/AllProjectiles.png");
    Texture2D texBlood = LoadTexture("assets/sprites/effects/blood_strip.png");
    Texture2D texGandalf = LoadTexture("assets/sprites/gandalf.png");

    std::vector<Texture2D> rohirrimFrames;
    rohirrimFrames.push_back(LoadTexture("assets/sprites/Knight_gallop1.png"));
    rohirrimFrames.push_back(LoadTexture("assets/sprites/Knight_gallop2.png"));
    rohirrimFrames.push_back(LoadTexture("assets/sprites/Knight_gallop3.png"));
    rohirrimFrames.push_back(LoadTexture("assets/sprites/Knight_gallop4.png"));
    rohirrimFrames.push_back(LoadTexture("assets/sprites/Knight_gallop5.png"));

    Camera2D camera = { 0 }; camera.zoom = 1.0f;
    std::vector<LevelData> allLevels;

    // ============================================
    // LEVEL 1: NORMAL (30 Sütun)
    // ============================================
    {
        LevelData lvl; lvl.levelID = 1; lvl.name = "Level 1: Outskirts (Short)";
        lvl.background = LoadTexture("assets/sprites/environment/minastirith_bg.png");
        lvl.bgColor = DARKGREEN; lvl.startGold = 400;

        // ÖNEMLİ: Bu level için genişlik 30
        int width = 30;
        lvl.cols = width;
        lvl.mapWidth = width * TILE_SIZE;
        lvl.castlePos = { 1400.0f, 50.0f }; lvl.castleScale = 1.0f;

        // Harita Tasarımı (30 Sütun)
        int design[12][30] = {
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {2,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,3,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
         };

        // Veriyi Vektöre Çevir
        lvl.tileMap.resize(MAP_ROWS);
        for (int y = 0; y < MAP_ROWS; y++) {
            lvl.tileMap[y].resize(width);
            for (int x = 0; x < width; x++) lvl.tileMap[y][x] = design[y][x];
        }

        lvl.paths = GeneratePathsFromMap(lvl.tileMap);
        lvl.waves.push_back({ 5, EnemyType::ORC, 1.5f, 1.0f, 0 });
        lvl.waves.push_back({ 8, EnemyType::ORC, 1.2f, 1.0f, 0 });
        lvl.waves.push_back({ 3, EnemyType::URUK, 2.0f, 1.0f, 0 });
        allLevels.push_back(lvl);
    }

    // ============================================
    // LEVEL 2: UZUN HARİTA (30 Sütun)
    // ============================================
    {
        LevelData lvl; lvl.levelID = 2; lvl.name = "Level 2: Long Road";
        lvl.background = LoadTexture("assets/sprites/environment/lvl2_bg.png");
        lvl.bgColor = DARKGRAY; lvl.startGold = 500;

        // ÖNEMLİ: Bu level için genişlik 50
        int width = 30;
        lvl.cols = width;
        lvl.mapWidth = width * TILE_SIZE;
        lvl.castlePos = { 1500.0f, 50.0f }; lvl.castleScale = 1.0f;

        // Harita Tasarımı (30 Sütunluk Dizi)
        int design[12][30] = {
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {2,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,3,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
        };

        lvl.tileMap.resize(MAP_ROWS);
        for (int y = 0; y < MAP_ROWS; y++) {
            lvl.tileMap[y].resize(width);
            for (int x = 0; x < width; x++) lvl.tileMap[y][x] = design[y][x];
        }

        lvl.paths = GeneratePathsFromMap(lvl.tileMap);
        lvl.waves.push_back({ 10, EnemyType::ORC, 1.0f, 1.1f, 5 });
        lvl.waves.push_back({ 5, EnemyType::URUK, 1.5f, 1.0f, 10 });
        lvl.waves.push_back({ 1, EnemyType::TROLL, 5.0f, 1.0f, 0 });
        allLevels.push_back(lvl);
    }

    // ============================================
    // LEVEL 3: ZOR (50 Sütun)
    // ============================================
    {
        LevelData lvl; lvl.levelID = 1; lvl.name = "Level 3: ZOR";
        lvl.background = LoadTexture("assets/sprites/environment/lvl3_bg.png");
        lvl.bgColor = DARKGREEN; lvl.startGold = 400;

        // ÖNEMLİ: Bu level için genişlik 50
        int width = 50;
        lvl.cols = width;
        lvl.mapWidth = width * TILE_SIZE;
        lvl.castlePos = { 2700.0f, 0.0f }; lvl.castleScale = 1.0f;

        // Harita Tasarımı (50 Sütun)
        int design[12][50] = {
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {2,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,0,0,0,0,0,0},
            {2,1,1,1,1,1,1,1,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
            };

        // Veriyi Vektöre Çevir
        lvl.tileMap.resize(MAP_ROWS);
        for (int y = 0; y < MAP_ROWS; y++) {
            lvl.tileMap[y].resize(width);
            for (int x = 0; x < width; x++) lvl.tileMap[y][x] = design[y][x];
        }

        lvl.paths = GeneratePathsFromMap(lvl.tileMap);
        lvl.waves.push_back({ 70, EnemyType::ORC, 0.5f, 1.5f, 0 });
        lvl.waves.push_back({ 70, EnemyType::ORC, 0.5f, 1.5f, 0 });
        lvl.waves.push_back({ 70, EnemyType::URUK, 0.5f, 1.5f, 0 });
        allLevels.push_back(lvl);
    }

    // Diğer levellerı buraya ekleyebilirsin...

    LevelData* currentLevel = nullptr;
    GameScreen currentScreen = GameScreen::TITLE;
    std::vector<Enemy> enemies;
    std::vector<Tower> towers;
    std::vector<Projectile> projectiles;
    std::vector<Rohirrim> riders;
    BloodManager bloodSystem; bloodSystem.Init(texBlood, 4);

    float spawnTimer = 0.0f;
    int gold = 0;

    int currentWaveIndex = 0;
    int enemiesSpawnedInWave = 0;
    float waveDelayTimer = 0.0f;

    TowerType selectedTower = TowerType::ARCHER;
    int urukBlood = 0;
    float flashTimer = 0.0f;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        Vector2 mouseScreenPos = GetMousePosition();
        Vector2 mouseWorldPos = GetScreenToWorld2D(mouseScreenPos, camera);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (currentScreen)
        {
        case GameScreen::TITLE:
            if (texMenuBg.id > 0) DrawTexturePro(texMenuBg, { 0,0,(float)texMenuBg.width, (float)texMenuBg.height }, { 0,0,(float)screenWidth, (float)screenHeight }, { 0,0 }, 0, WHITE);
            else DrawRectangleGradientV(0, 0, screenWidth, screenHeight, DARKBLUE, BLACK);
            DrawText("SIEGE OF GONDOR", screenWidth / 2 - MeasureText("SIEGE OF GONDOR", 60) / 2, 150, 60, GOLD);
            if (GuiButton({ (float)screenWidth / 2 - 100, 400, 200, 50 }, "PLAY GAME", texBtnNormal, texBtnHover)) currentScreen = GameScreen::LEVEL_SELECT;
            if (GuiButton({ (float)screenWidth / 2 - 100, 470, 200, 50 }, "EXIT", texBtnNormal, texBtnHover)) { CloseWindow(); return 0; }
            break;

        case GameScreen::LEVEL_SELECT:
            if (texMenuBg.id > 0) DrawTexturePro(texMenuBg, { 0,0,(float)texMenuBg.width, (float)texMenuBg.height }, { 0,0,(float)screenWidth, (float)screenHeight }, { 0,0 }, 0, GRAY);
            else DrawRectangle(0, 0, screenWidth, screenHeight, DARKGRAY);
            DrawText("SELECT A BATTLEFIELD", screenWidth / 2 - MeasureText("SELECT A BATTLEFIELD", 40) / 2, 50, 40, WHITE);
            {
                int btnWidth = 300; int btnHeight = 60; int gap = 10; int startY = 120;
                for (int i = 0; i < allLevels.size(); i++) {
                    float x = (float)screenWidth / 2 - btnWidth / 2;
                    float y = (float)startY + i * (btnHeight + gap);
                    if (GuiButton({ x, y, (float)btnWidth, (float)btnHeight }, allLevels[i].name, texBtnNormal, texBtnHover)) {
                        currentLevel = &allLevels[i];
                        gold = currentLevel->startGold;
                        enemies.clear(); towers.clear(); projectiles.clear(); riders.clear(); bloodSystem.particles.clear();
                        currentWaveIndex = 0; enemiesSpawnedInWave = 0; waveDelayTimer = 0.0f; urukBlood = 0;
                        camera.target = { 0, 0 };
                        currentScreen = GameScreen::GAMEPLAY;
                    }
                }
            }
            if (GuiButton({ 20, 20, 100, 40 }, "BACK", texBtnNormal, texBtnHover)) currentScreen = GameScreen::TITLE;
            break;

        case GameScreen::GAMEPLAY:
            if (currentLevel->mapWidth > screenWidth) {
                float scrollSpeed = 10.0f; float edgeSize = 50.0f;
                if (mouseScreenPos.x >= screenWidth - edgeSize) camera.target.x += scrollSpeed;
                if (mouseScreenPos.x <= edgeSize) camera.target.x -= scrollSpeed;
                if (camera.target.x < 0) camera.target.x = 0;
                if (camera.target.x > currentLevel->mapWidth - screenWidth) camera.target.x = currentLevel->mapWidth - screenWidth;
            }

            if (IsKeyPressed(KEY_Q)) { if (urukBlood >= COST_GANDALF) { urukBlood -= COST_GANDALF; flashTimer = 1.0f; for (Enemy& e : enemies) e.ApplyStun(3.0f); } }
            if (IsKeyPressed(KEY_W)) { if (urukBlood >= COST_ROHIRRIM) { urukBlood -= COST_ROHIRRIM; for (auto* path : currentLevel->paths) riders.emplace_back(path, &rohirrimFrames); } }
            if (IsKeyPressed(KEY_ONE))   selectedTower = TowerType::ARCHER;
            if (IsKeyPressed(KEY_TWO))   selectedTower = TowerType::MELEE;
            if (IsKeyPressed(KEY_THREE)) selectedTower = TowerType::ICE;

            int gridX = (int)(mouseWorldPos.x / TILE_SIZE);
            int gridY = (int)(mouseWorldPos.y / TILE_SIZE);
            Vector2 snapPos = { (float)gridX * TILE_SIZE + TILE_SIZE / 2, (float)gridY * TILE_SIZE + TILE_SIZE / 2 };

            bool isValidPlacement = true;
            bool isHoveringUI = (mouseScreenPos.y > screenHeight - 60);

            // DİNAMİK COLS KULLANIMI
            if (gridX < 0 || gridX >= currentLevel->cols || gridY < 0 || gridY >= MAP_ROWS) isValidPlacement = false;
            else if (currentLevel->tileMap[gridY][gridX] != 0) isValidPlacement = false;

            if (isHoveringUI) isValidPlacement = false;
            else {
                if (gold < GetTowerCost(selectedTower)) isValidPlacement = false;
                for (const Tower& t : towers) if (CheckCollisionPointCircle(snapPos, t.GetPosition(), 10.0f)) isValidPlacement = false;
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                bool clickedExisting = false;
                for (Tower& t : towers) {
                    if (t.IsClicked(mouseWorldPos)) {
                        clickedExisting = true;
                        if (gold >= t.GetUpgradeCost()) { gold -= t.GetUpgradeCost(); t.Upgrade(); }
                        break;
                    }
                }
                if (!clickedExisting && isValidPlacement) {
                    Texture2D textureToUse = texTowerArcher;
                    if (selectedTower == TowerType::MELEE) textureToUse = texTowerMelee;
                    else if (selectedTower == TowerType::ICE) textureToUse = texTowerIce;
                    towers.emplace_back(snapPos, textureToUse, texProjectiles, selectedTower);
                    gold -= GetTowerCost(selectedTower);
                }
            }

            if (currentWaveIndex < currentLevel->waves.size()) {
                EnemyWave& w = currentLevel->waves[currentWaveIndex];
                if (enemiesSpawnedInWave < w.enemyCount) {
                    spawnTimer += dt;
                    if (spawnTimer >= w.spawnInterval) {
                        spawnTimer = 0.0f;
                        if (!currentLevel->paths.empty()) {
                            int pathIndex = GetRandomValue(0, (int)currentLevel->paths.size() - 1);
                            std::vector<Vector2>* chosenPath = currentLevel->paths[pathIndex];
                            enemies.push_back(Enemy(w.enemyType, chosenPath,
                                (w.enemyType == EnemyType::ORC ? texOrc : (w.enemyType == EnemyType::URUK ? texUruk : texTroll)),
                                w.speedMultiplier, w.healthBonus));
                            enemiesSpawnedInWave++;
                        }
                    }
                }
                else if (enemies.empty()) {
                    waveDelayTimer += dt;
                    if (waveDelayTimer > 3.0f) { currentWaveIndex++; enemiesSpawnedInWave = 0; waveDelayTimer = 0.0f; }
                }
            }
            else {
                if (enemies.empty()) DrawText("VICTORY!", screenWidth / 2 - 100, screenHeight / 2, 60, GOLD);
            }

            bloodSystem.Update(dt);
            for (int i = 0; i < enemies.size(); i++) {
                enemies[i].Update(dt);
                if (!enemies[i].IsAlive()) { gold += 15; urukBlood += enemies[i].GetManaReward(); if (urukBlood > MAX_BLOOD) urukBlood = MAX_BLOOD; enemies.erase(enemies.begin() + i); i--; continue; }
                if (enemies[i].ReachedEnd()) { enemies.erase(enemies.begin() + i); i--; }
            }
            for (int i = 0; i < riders.size(); i++) {
                riders[i].Update(dt);
                for (Enemy& e : enemies) {
                    if (e.IsAlive() && CheckCollisionCircles(riders[i].position, 30, e.GetPosition(), 20)) {
                        e.TakeDamage(100); bloodSystem.Spawn(e.GetPosition());
                        if (!e.IsAlive()) { urukBlood += e.GetManaReward(); if (urukBlood > MAX_BLOOD) urukBlood = MAX_BLOOD; }
                    }
                }
                if (!riders[i].active) { riders.erase(riders.begin() + i); i--; }
            }
            for (Tower& t : towers) t.Update(dt, enemies, projectiles);
            for (int i = 0; i < projectiles.size(); i++) {
                projectiles[i].Update(dt);
                for (Enemy& e : enemies) {
                    if (e.IsAlive() && CheckCollisionCircles(projectiles[i].position, 5, e.GetPosition(), e.GetRadius())) {
                        e.TakeDamage(projectiles[i].damage); bloodSystem.Spawn(e.GetPosition());
                        if (projectiles[i].type == ProjectileType::ICE) e.ApplySlow(0.5f, 2.0f);
                        projectiles[i].active = false;
                        if (!e.IsAlive()) { gold += 15; urukBlood += e.GetManaReward(); if (urukBlood > MAX_BLOOD) urukBlood = MAX_BLOOD; }
                        break;
                    }
                }
                if (!projectiles[i].active) { projectiles.erase(projectiles.begin() + i); i--; }
            }

            BeginMode2D(camera);

            // --- ARKA PLAN ÇİZİMİ (DÜZELTİLDİ: REPEAT MODU) ---
            if (currentLevel->background.id > 0) {

                // 1. Resmi "Tekrar Et" moduna al
                SetTextureWrap(currentLevel->background, TEXTURE_WRAP_REPEAT);

                // 2. Çizim Parametreleri
                // Kaynak (Source) Genişliği: Resmin genişliği değil, HARİTANIN genişliği kadar istiyoruz.
                // Raylib, resim bittiğinde Repeat modunda olduğu için başa dönüp tekrar çizecektir.

                Rectangle sourceRec = {
                    0,
                    0,
                    (float)currentLevel->mapWidth,          // <-- PÜF NOKTASI: Harita genişliği kadar kaynak al
                    (float)currentLevel->background.height  // Yükseklik resim kadar olsun
                };

                Rectangle destRec = {
                    0,
                    0,
                    (float)currentLevel->mapWidth,
                    (float)screenHeight
                };

                DrawTexturePro(currentLevel->background, sourceRec, destRec, { 0, 0 }, 0.0f, WHITE);
            }
            else {
                DrawRectangle(0, 0, currentLevel->mapWidth, screenHeight, currentLevel->bgColor);
            }

            // DİNAMİK COLS KULLANIMI
            for (int y = 0; y < MAP_ROWS; y++) {
                for (int x = 0; x < currentLevel->cols; x++) {
                    Rectangle destRect = { (float)x * TILE_SIZE, (float)y * TILE_SIZE, (float)TILE_SIZE, (float)TILE_SIZE };

                    if (currentLevel->tileMap[y][x] == 1 || currentLevel->tileMap[y][x] == 2 || currentLevel->tileMap[y][x] == 3) {
                        if (texRoad.id > 0) DrawTexturePro(texRoad, { 0,0,(float)texRoad.width,(float)texRoad.height }, destRect, { 0,0 }, 0, WHITE);
                        else DrawRectangleRec(destRect, BROWN);
                    }
                }
            }

            if (texCity.id > 0) {
                DrawTexturePro(texCity,
                    { 0, 0, (float)texCity.width, (float)texCity.height },
                        {
                            currentLevel->castlePos.x,
                            currentLevel->castlePos.y,
                            (float)texCity.width * currentLevel->castleScale,
                            (float)texCity.height * currentLevel->castleScale
                        },
                    { 0, 0 }, 0.0f, WHITE);
            }

            for (const auto& t : towers) t.Draw();
            for (const auto& e : enemies) e.Draw();
            for (auto& r : riders) r.Draw();
            bloodSystem.Draw();
            for (const auto& p : projectiles) p.Draw();

            bool hoverExisting = false;
            for (const Tower& t : towers) {
                if (t.IsClicked(mouseWorldPos)) {
                    hoverExisting = true;
                    DrawText(TextFormat("UPGRADE: %dg", t.GetUpgradeCost()), (int)mouseWorldPos.x, (int)mouseWorldPos.y - 40, 20, GREEN);
                    DrawCircleLines((int)t.GetPosition().x, (int)t.GetPosition().y, t.GetRange(), GREEN);
                }
            }
            if (!hoverExisting && !isHoveringUI) {
                float range = GetTowerRange(selectedTower);
                Color c = isValidPlacement ? Fade(GREEN, 0.5f) : Fade(RED, 0.5f);
                Color border = isValidPlacement ? GREEN : RED;
                DrawCircleV(snapPos, range, c);
                DrawCircleLines((int)snapPos.x, (int)snapPos.y, range, border);
            }

            EndMode2D();

            if (flashTimer > 0.0f) {
                flashTimer -= dt;
                DrawRectangle(0, 0, screenWidth, screenHeight, Fade(WHITE, flashTimer * 0.4f));
                float cellSize = 64.0f;
                int animFrame = (int)((1.0f - flashTimer) * 8.0f) % 7;
                Rectangle source = { animFrame * cellSize, 2 * cellSize, cellSize, cellSize };
                DrawTexturePro(texGandalf, source, { (float)screenWidth / 2 - 64, (float)screenHeight / 2 - 80, 128, 128 }, { 0,0 }, 0.0f, Fade(WHITE, flashTimer + 0.5f));
            }

            DrawRectangle(0, screenHeight - 60, screenWidth, 60, Fade(BLACK, 0.9f));
            DrawText(TextFormat("Gold: %d", gold), 20, screenHeight - 45, 20, YELLOW);
            DrawText(TextFormat("Wave: %d / %d", currentWaveIndex + 1, currentLevel->waves.size()), 20, screenHeight - 25, 20, WHITE);

            Color c1 = (selectedTower == TowerType::ARCHER) ? YELLOW : GRAY;
            Color c2 = (selectedTower == TowerType::MELEE) ? RED : GRAY;
            Color c3 = (selectedTower == TowerType::ICE) ? SKYBLUE : GRAY;
            DrawText("[1] Archer", 400, screenHeight - 40, 20, c1);
            DrawText("[2] Melee", 550, screenHeight - 40, 20, c2);
            DrawText("[3] Ice", 700, screenHeight - 40, 20, c3);

            int rightX = screenWidth - 450; int barY = screenHeight - 35;
            DrawRectangleLines(rightX, barY, 120, 15, GRAY);
            float bloodPct = (float)urukBlood / MAX_BLOOD;
            DrawRectangle(rightX + 1, barY + 1, (int)(118 * bloodPct), 13, MAROON);
            DrawText("BLOOD", rightX + 40, barY + 3, 10, WHITE);
            Color cQ = (urukBlood >= COST_GANDALF) ? WHITE : DARKGRAY;
            Color cW = (urukBlood >= COST_ROHIRRIM) ? WHITE : DARKGRAY;
            DrawText("[Q] GANDALF", rightX + 130, barY - 5, 20, cQ);
            DrawText("[W] ROHIRRIM", rightX + 280, barY - 5, 20, cW);

            if (GuiButton({ (float)screenWidth - 120, 10, 100, 30 }, "MENU", texBtnNormal, texBtnHover)) {
                currentScreen = GameScreen::TITLE;
            }
            break;
        }

        EndDrawing();
    }

    UnloadTexture(texOrc); UnloadTexture(texUruk); UnloadTexture(texTroll);
    UnloadTexture(texTowerArcher); UnloadTexture(texTowerMelee); UnloadTexture(texTowerIce);
    UnloadTexture(texMenuBg); UnloadTexture(texBtnNormal); UnloadTexture(texBtnHover);
    UnloadTexture(texProjectiles); UnloadTexture(texBlood); UnloadTexture(texRoad); UnloadTexture(texCity);
    UnloadTexture(texGandalf);
    for (auto& lvl : allLevels) {
        UnloadTexture(lvl.background);
        for (auto* p : lvl.paths) delete p;
    }
    CloseWindow();
    return 0;
}