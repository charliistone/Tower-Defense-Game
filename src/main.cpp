#include "raylib.h"
#include "raymath.h"
#include "Enemy.h"
#include "Tower.h"      
#include "Projectile.h"  
#include "Audio.h" 
#include <vector>
#include <string>
#include <algorithm> 


#define MIN(a, b) ((a)<(b)?(a):(b))


enum class GameScreen { TITLE, LEVEL_SELECT, LEVEL_INTRO, GAMEPLAY, VICTORY, GAMEOVER };


const int TILE_SIZE = 64;
const int MAP_ROWS = 12;


struct EnemyWave {
    int enemyCount;
    EnemyType enemyType;
    float spawnInterval;
    float speedMultiplier;
    int healthBonus;
};


struct LevelData {
    int levelID;
    const char* name;
    Texture2D background;
    Color bgColor;

    std::vector<std::vector<int>> tileMap;
    std::vector<std::vector<Vector2>*> paths;
    int startGold;
    int mapWidth;
    int cols;

    Vector2 castlePos;
    float castleScale;
    std::vector<EnemyWave> waves;

    std::vector<std::string> storyLines;
};


void FindAllPathsRecursive(int x, int y, int cols, int rows,
    std::vector<std::vector<int>>& tileMap,
    std::vector<Vector2> currentPath,
    std::vector<std::vector<Vector2>*>& outPaths)
{
    currentPath.push_back({ (float)x * TILE_SIZE + TILE_SIZE / 2, (float)y * TILE_SIZE + TILE_SIZE / 2 });

    if (tileMap[y][x] == 3) {
        outPaths.push_back(new std::vector<Vector2>(currentPath));
        return;
    }

    int originalValue = tileMap[y][x];
    tileMap[y][x] = -1;

    int dx[] = { 1, 0, -1, 0 };
    int dy[] = { 0, 1, 0, -1 };

    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];

        if (nx >= 0 && nx < cols && ny >= 0 && ny < rows) {
            if (tileMap[ny][nx] == 1 || tileMap[ny][nx] == 3) {
                FindAllPathsRecursive(nx, ny, cols, rows, tileMap, currentPath, outPaths);
            }
        }
    }
    tileMap[y][x] = originalValue;
}


std::vector<std::vector<Vector2>*> GeneratePathsFromMap(const std::vector<std::vector<int>>& map) {
    std::vector<std::vector<Vector2>*> allPaths;
    if (map.empty()) return allPaths;

    int rows = map.size();
    int cols = map[0].size();
    std::vector<std::vector<int>> tempMap = map;

    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            if (tempMap[y][x] == 2) {
                std::vector<Vector2> initialPath;
                FindAllPathsRecursive(x, y, cols, rows, tempMap, initialPath, allPaths);
            }
        }
    }
    return allPaths;
}


bool GuiButton(Rectangle rect, const char* text, Texture2D texNormal, Texture2D texHover, Vector2 mousePos) {
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

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                clicked = true;
                Audio::PlaySFX("ui_click");
            }
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

float GetTowerRange(TowerType type) {
    if (type == TowerType::ARCHER) return 250.0f;
    if (type == TowerType::ICE) return 180.0f;
    return 100.0f;
}

int GetTowerCost(TowerType type) {
    if (type == TowerType::ARCHER) return 100;
    if (type == TowerType::ICE) return 150;
    return 100;
}

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


int main(void)
{
    
    const int gameScreenWidth = 1280;
    const int gameScreenHeight = 720;

   
    int screenWidth = 1280;
    int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Siege of Gondor - Master Edition");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    Audio::Init();

   
    RenderTexture2D target = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

   
    Audio::LoadMusic("music_menu", "assets/sounds/menu_theme.mp3");
    Audio::LoadMusic("music_level1", "assets/sounds/music_level1.mp3");
    Audio::LoadMusic("music_level2", "assets/sounds/music_level2.wav");
    Audio::LoadMusic("music_level3", "assets/sounds/music_level3.mp3");
    Audio::LoadMusic("game_over", "assets/sounds/game_over.mp3");
    Audio::LoadMusic("victory_jingle", "assets/sounds/victory_jingle.wav");
    Audio::LoadMusic("music_boss", "assets/sounds/nazgul_boss.mp3");

    Audio::LoadSFX("arrow_shoot", "assets/sounds/arrow_shoot.wav");
    Audio::LoadSFX("ice_shoot", "assets/sounds/ice_shoot.wav");
    Audio::LoadSFX("arrow_hit", "assets/sounds/arrow_hit.wav");
    Audio::LoadSFX("ice_hit", "assets/sounds/ice_hit.wav");
    Audio::LoadSFX("magic_shoot", "assets/sounds/magic.wav");
    Audio::LoadSFX("sword_hit", "assets/sounds/spear_hit.wav");
    Audio::LoadSFX("orc_death", "assets/sounds/orc_death.wav");
    Audio::LoadSFX("build_tower", "assets/sounds/build.wav");
    Audio::LoadSFX("ui_click", "assets/sounds/click.wav");
    Audio::LoadSFX("gold_gain", "assets/sounds/gold_gain.wav");

    Audio::LoadSFX("spawn_1", "assets/sounds/orc_spawn_1.wav");
    Audio::LoadSFX("spawn_2", "assets/sounds/orc_spawn_2.wav");
    Audio::LoadSFX("spawn_3", "assets/sounds/orc_spawn_3.wav");

    Audio::LoadSFX("gandalf", "assets/sounds/gandalf_spell.wav");
    Audio::LoadSFX("rohirrim", "assets/sounds/rohirrim_charge.mp3");
    Audio::LoadSFX("victory", "assets/sounds/victory.wav");

    Audio::LoadSFX("orc_walk", "assets/sounds/orc_walk.wav");
    Audio::LoadSFX("heavy_walk", "assets/sounds/grond_walk.wav");

   
    Texture2D texMenuBg = LoadTexture("assets/ui/menu_bg.png");
    Texture2D texVictoryBg = LoadTexture("assets/ui/victory_bg.png");
    Texture2D texDefeatBg = LoadTexture("assets/ui/defeat_bg.png");

    Texture2D texBtnNormal = LoadTexture("assets/ui/btn_default.png");
    Texture2D texBtnHover = LoadTexture("assets/ui/btn_hover.png");

    Texture2D texOrc = LoadTexture("assets/sprites/enemies/orc.png");
    Texture2D texUruk = LoadTexture("assets/sprites/enemies/uruk.png");
    Texture2D texTroll = LoadTexture("assets/sprites/enemies/troll.png");
    Texture2D texGrond = LoadTexture("assets/sprites/enemies/grond.png");
    Texture2D texCommander = LoadTexture("assets/sprites/enemies/commander.png");
    Texture2D texNazgul = LoadTexture("assets/sprites/enemies/nazgul.png");

    Texture2D texRoad = LoadTexture("assets/sprites/environment/road_texture.png");
    Texture2D texCity = LoadTexture("assets/sprites/environment/minastirith_city.png");

    Texture2D texTowerArcher = LoadTexture("assets/sprites/towers/tower_archer.png");
    Texture2D texTowerMelee = LoadTexture("assets/sprites/towers/tower_melee.png");
    Texture2D texTowerIce = LoadTexture("assets/sprites/towers/tower_ice.png");

    Texture2D texProjArrow = LoadTexture("assets/sprites/projectiles/arrow_sheet.png");
    Texture2D texProjIce = LoadTexture("assets/sprites/projectiles/ice_sheet.png");
    Texture2D texProjMelee = LoadTexture("assets/sprites/projectiles/melee_fx.png");

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

   
    {
        LevelData lvl; lvl.levelID = 1; lvl.name = "Level 1: Outskirts";
        lvl.background = LoadTexture("assets/sprites/environment/minastirith_bg.png");
        lvl.bgColor = DARKGREEN;
        lvl.startGold = 400;
        int width = 30; lvl.cols = width; lvl.mapWidth = width * TILE_SIZE;
        lvl.castlePos = { 1550.0f, 120.0f }; lvl.castleScale = 0.5f;
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
        lvl.tileMap.resize(MAP_ROWS);
        for (int y = 0; y < MAP_ROWS; y++) { lvl.tileMap[y].resize(width); for (int x = 0; x < width; x++) lvl.tileMap[y][x] = design[y][x]; }
        lvl.paths = GeneratePathsFromMap(lvl.tileMap);

        lvl.waves.push_back({ 5, EnemyType::ORC, 1.5f, 1.0f, 0 });
        lvl.waves.push_back({ 12, EnemyType::ORC, 1.0f, 1.1f, 0 });
        lvl.waves.push_back({ 5, EnemyType::URUK, 2.0f, 0.9f, 10 });
        lvl.waves.push_back({ 15, EnemyType::ORC, 0.8f, 1.2f, 5 });
        lvl.waves.push_back({ 15, EnemyType::URUK, 1.5f, 1.0f, 20 });

        lvl.storyLines = {
            "CHAPTER 1: THE OUTSKIRTS",
            "Scouts report Orc activity near the farm lands.",
            "They are testing our defenses.",
            "Hold them off before they reach the main gate."
        };
        allLevels.push_back(lvl);
    }

    
    {
        LevelData lvl; lvl.levelID = 2; lvl.name = "Level 2: Long Road";
        lvl.background = LoadTexture("assets/sprites/environment/lvl2_bg.png");
        lvl.bgColor = DARKGREEN;
        lvl.startGold = 600;
        int width = 30; lvl.cols = width; lvl.mapWidth = width * TILE_SIZE;
        lvl.castlePos = { 1550.0f, 120.0f }; lvl.castleScale = 0.5f;
        int design[12][30] = {
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {2,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,0,0,0,0},
            {2,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
        };
        lvl.tileMap.resize(MAP_ROWS);
        for (int y = 0; y < MAP_ROWS; y++) { lvl.tileMap[y].resize(width); for (int x = 0; x < width; x++) lvl.tileMap[y][x] = design[y][x]; }
        lvl.paths = GeneratePathsFromMap(lvl.tileMap);

        lvl.waves.push_back({ 10, EnemyType::ORC, 1.0f, 1.1f, 5 });
        lvl.waves.push_back({ 15, EnemyType::ORC, 0.9f, 1.2f, 10 });
        lvl.waves.push_back({ 8, EnemyType::URUK, 1.5f, 1.0f, 10 });
        lvl.waves.push_back({ 2, EnemyType::COMMANDER, 2.0f, 1.0f, 0 });
        lvl.waves.push_back({ 4, EnemyType::TROLL, 5.0f, 1.0f, 20 });

        lvl.storyLines = {
            "CHAPTER 2: THE LONG ROAD",
            "Osgiliath has fallen.",
            "The enemy is marching towards the Great River.",
            "Ambush their vanguards on the road!",
            "Beware of the Trolls."
        };
        allLevels.push_back(lvl);
    }

    
    {
        LevelData lvl; lvl.levelID = 3; lvl.name = "Level 3: The Siege";
        lvl.background = LoadTexture("assets/sprites/environment/lvl3_bg.png");
        lvl.bgColor = DARKGREEN;
        lvl.startGold = 700;
        int width = 50; lvl.cols = width; lvl.mapWidth = width * TILE_SIZE;
        lvl.castlePos = { 2700.0f, 90.0f }; lvl.castleScale = 0.5f;
        int design[12][50] = {
            {0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {2,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,0,0,0,0,0,0},
            {2,1,1,1,1,1,1,1,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
        };
        lvl.tileMap.resize(MAP_ROWS);
        for (int y = 0; y < MAP_ROWS; y++) { lvl.tileMap[y].resize(width); for (int x = 0; x < width; x++) lvl.tileMap[y][x] = design[y][x]; }
        lvl.paths = GeneratePathsFromMap(lvl.tileMap);

        lvl.waves.push_back({ 20, EnemyType::ORC, 0.8f, 1.0f, 10 });
        lvl.waves.push_back({ 15, EnemyType::URUK, 1.0f, 1.1f, 20 });
        lvl.waves.push_back({ 25, EnemyType::ORC, 0.6f, 1.1f, 15 });
        lvl.waves.push_back({ 6,  EnemyType::TROLL, 4.0f, 1.0f, 50 });
        lvl.waves.push_back({ 25, EnemyType::URUK, 0.7f, 1.3f, 30 }); 
        lvl.waves.push_back({ 5, EnemyType::COMMANDER, 2.0f, 1.0f, 0 });
        lvl.waves.push_back({ 10,  EnemyType::TROLL, 3.0f, 1.1f, 60 });
        lvl.waves.push_back({ 1,  EnemyType::NAZGUL, 5.0f, 1.5f, 500 });

        lvl.storyLines = {
            "CHAPTER 3: THE SIEGE BEGINS",
            "The sky turns red as the Great Eye turns toward us.",
            "The Witch King comes to claim Gondor.",
            "This is our final stand.",
            "YOU SHALL NOT PASS!"
        };
        allLevels.push_back(lvl);
    }

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
    float bossLabelTimer = 0.0f;

    int castleHealth = 100;
    bool isBossActive = false;
    const int CASTLE_MAX_HEALTH = 100;

    float walkSoundTimer = 0.0f;
    float introAlpha = 0.0f;
    int introState = 0;
    float introTimer = 0.0f;
    int introTextIndex = 0;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        
        float scale = MIN((float)GetScreenWidth() / gameScreenWidth, (float)GetScreenHeight() / gameScreenHeight);

       
        Vector2 mouseRaw = GetMousePosition();
        Vector2 virtualMouse = { 0 };
        float offsetX = (GetScreenWidth() - (gameScreenWidth * scale)) * 0.5f;
        float offsetY = (GetScreenHeight() - (gameScreenHeight * scale)) * 0.5f;
        virtualMouse.x = (mouseRaw.x - offsetX) / scale;
        virtualMouse.y = (mouseRaw.y - offsetY) / scale;
        virtualMouse = Vector2Clamp(virtualMouse, { 0, 0 }, { (float)gameScreenWidth, (float)gameScreenHeight });

        Vector2 mouseScreenPos = virtualMouse;
        Vector2 mouseWorldPos = GetScreenToWorld2D(mouseScreenPos, camera);

        
        if (IsKeyPressed(KEY_F11)) {
            int monitor = GetCurrentMonitor();
            if (IsWindowFullscreen()) {
                ToggleFullscreen();
                SetWindowSize(gameScreenWidth, gameScreenHeight);
            }
            else {
                SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
                ToggleFullscreen();
            }
        }

        Audio::Update();

        
        BeginTextureMode(target);
        ClearBackground(RAYWHITE);

        
        switch (currentScreen)
        {
        case GameScreen::TITLE:
        case GameScreen::LEVEL_SELECT:
        case GameScreen::LEVEL_INTRO:
            Audio::PlayMusic("music_menu");
            Audio::SetMusicVolume(0.3f);
            break;

        case GameScreen::GAMEPLAY:
            if (isBossActive) {
                Audio::PlayMusic("music_boss");
                Audio::SetMusicVolume(0.4f);
            }
            else {
                if (currentLevel->levelID == 1) Audio::PlayMusic("music_level1");
                else if (currentLevel->levelID == 2) Audio::PlayMusic("music_level2");
                else if (currentLevel->levelID == 3) Audio::PlayMusic("music_level3");
                Audio::SetMusicVolume(0.2f);
            }
            break;

        case GameScreen::VICTORY:
            Audio::PlayMusic("victory_jingle");
            break;

        case GameScreen::GAMEOVER:
            Audio::PlayMusic("game_over");
            break;
        }

      
        switch (currentScreen)
        {
        case GameScreen::TITLE:
            if (texMenuBg.id > 0) DrawTexturePro(texMenuBg, { 0,0,(float)texMenuBg.width, (float)texMenuBg.height }, { 0,0,(float)gameScreenWidth, (float)gameScreenHeight }, { 0,0 }, 0, WHITE);
            else DrawRectangleGradientV(0, 0, gameScreenWidth, gameScreenHeight, DARKBLUE, BLACK);
            DrawText("SIEGE OF GONDOR", gameScreenWidth / 2 - MeasureText("SIEGE OF GONDOR", 60) / 2, 150, 60, GOLD);

           
            if (GuiButton({ (float)gameScreenWidth / 2 - 100, 400, 200, 50 }, "PLAY GAME", texBtnNormal, texBtnHover, mouseScreenPos)) currentScreen = GameScreen::LEVEL_SELECT;
            if (GuiButton({ (float)gameScreenWidth / 2 - 100, 470, 200, 50 }, "EXIT", texBtnNormal, texBtnHover, mouseScreenPos)) { CloseWindow(); return 0; }
            break;

        case GameScreen::LEVEL_SELECT:
            if (texMenuBg.id > 0) DrawTexturePro(texMenuBg, { 0,0,(float)texMenuBg.width, (float)texMenuBg.height }, { 0,0,(float)gameScreenWidth, (float)gameScreenHeight }, { 0,0 }, 0, GRAY);
            else DrawRectangle(0, 0, gameScreenWidth, gameScreenHeight, DARKGRAY);
            DrawText("SELECT A BATTLEFIELD", gameScreenWidth / 2 - MeasureText("SELECT A BATTLEFIELD", 40) / 2, 50, 40, WHITE);
            {
                int btnWidth = 300; int btnHeight = 60; int gap = 10; int startY = 120;
                for (int i = 0; i < allLevels.size(); i++) {
                    float x = (float)gameScreenWidth / 2 - btnWidth / 2;
                    float y = (float)startY + i * (btnHeight + gap);
                    
                    if (GuiButton({ x, y, (float)btnWidth, (float)btnHeight }, allLevels[i].name, texBtnNormal, texBtnHover, mouseScreenPos)) {
                        currentLevel = &allLevels[i];
                        gold = currentLevel->startGold;
                        castleHealth = CASTLE_MAX_HEALTH;
                        enemies.clear(); towers.clear(); projectiles.clear(); riders.clear(); bloodSystem.particles.clear();
                        currentWaveIndex = 0; enemiesSpawnedInWave = 0; waveDelayTimer = 0.0f; urukBlood = 0;
                        camera.target = { 0, 0 };
                        isBossActive = false;
                        currentScreen = GameScreen::LEVEL_INTRO;
                        introState = 0; introAlpha = 0.0f; introTimer = 0.0f; introTextIndex = 0;
                    }
                }
            }
            if (GuiButton({ 20, 20, 100, 40 }, "BACK", texBtnNormal, texBtnHover, mouseScreenPos)) currentScreen = GameScreen::TITLE;
            break;

        case GameScreen::LEVEL_INTRO:
        {
            ClearBackground(BLACK);
            if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) { currentScreen = GameScreen::GAMEPLAY; }

            float fadeSpeed = 0.5f; float holdTime = 2.5f;
            if (introState == 0) {
                introAlpha += fadeSpeed * dt;
                if (introAlpha >= 1.0f) { introAlpha = 1.0f; introState = 1; introTimer = 0.0f; }
            }
            else if (introState == 1) {
                introTimer += dt;
                if (introTimer >= holdTime) { introState = 2; }
            }
            else if (introState == 2) {
                introAlpha -= fadeSpeed * dt;
                if (introAlpha <= 0.0f) {
                    introAlpha = 0.0f; introState = 0; introTextIndex++;
                    if (introTextIndex >= currentLevel->storyLines.size()) { currentScreen = GameScreen::GAMEPLAY; }
                }
            }

            if (currentLevel && introTextIndex < currentLevel->storyLines.size()) {
                const char* text = currentLevel->storyLines[introTextIndex].c_str();
                int fontSize = 40; int textW = MeasureText(text, fontSize);
                DrawText(text, (gameScreenWidth - textW) / 2, (gameScreenHeight - fontSize) / 2, fontSize, Fade(WHITE, introAlpha));
            }
            DrawText("Press ENTER to Skip", gameScreenWidth - 250, gameScreenHeight - 40, 20, Fade(GRAY, 0.5f));
        }
        break;

        case GameScreen::GAMEPLAY:
        {
            if (currentLevel->mapWidth > gameScreenWidth) {
                float scrollSpeed = 10.0f; float edgeSize = 50.0f;
                if (mouseScreenPos.x >= gameScreenWidth - edgeSize) camera.target.x += scrollSpeed;
                if (mouseScreenPos.x <= edgeSize) camera.target.x -= scrollSpeed;
                if (camera.target.x < 0) camera.target.x = 0;
                if (camera.target.x > currentLevel->mapWidth - gameScreenWidth) camera.target.x = currentLevel->mapWidth - gameScreenWidth;
            }

            if (IsKeyPressed(KEY_Q)) {
                if (urukBlood >= COST_GANDALF) {
                    urukBlood -= COST_GANDALF;
                    flashTimer = 2.0f;
                    Audio::PlaySFX("gandalf");
                    for (Enemy& e : enemies) e.ApplyStun(3.0f);
                }
            }
            if (IsKeyPressed(KEY_W)) {
                if (urukBlood >= COST_ROHIRRIM) {
                    urukBlood -= COST_ROHIRRIM;
                    Audio::PlaySFX("rohirrim");
                    for (auto* path : currentLevel->paths) riders.emplace_back(path, &rohirrimFrames);
                }
            }

            if (IsKeyPressed(KEY_ONE))   selectedTower = TowerType::ARCHER;
            if (IsKeyPressed(KEY_TWO))   selectedTower = TowerType::MELEE;
            if (IsKeyPressed(KEY_THREE)) selectedTower = TowerType::ICE;

            int gridX = (int)(mouseWorldPos.x / TILE_SIZE);
            int gridY = (int)(mouseWorldPos.y / TILE_SIZE);
            Vector2 snapPos = { (float)gridX * TILE_SIZE + TILE_SIZE / 2, (float)gridY * TILE_SIZE + TILE_SIZE / 2 };

            bool isValidPlacement = true;
            bool isHoveringUI = (mouseScreenPos.y > gameScreenHeight - 60);

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
                    Texture2D projectileToUse = texProjArrow;
                    if (selectedTower == TowerType::MELEE) { textureToUse = texTowerMelee; projectileToUse = texProjMelee; }
                    else if (selectedTower == TowerType::ICE) { textureToUse = texTowerIce; projectileToUse = texProjIce; }

                    towers.emplace_back(snapPos, textureToUse, projectileToUse, selectedTower);
                    Audio::PlaySFX("build_tower");
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

                            Texture2D currentEnemyTex = texOrc;
                            if (w.enemyType == EnemyType::URUK) currentEnemyTex = texUruk;
                            else if (w.enemyType == EnemyType::TROLL) currentEnemyTex = texTroll;
                            else if (w.enemyType == EnemyType::GROND) currentEnemyTex = texGrond;
                            else if (w.enemyType == EnemyType::NAZGUL) currentEnemyTex = texNazgul;
                            else if (w.enemyType == EnemyType::COMMANDER) currentEnemyTex = texCommander;


                            int dynamicHealth = w.healthBonus + (currentLevel->levelID * 15);
                            enemies.push_back(Enemy(w.enemyType, chosenPath, currentEnemyTex, w.speedMultiplier, dynamicHealth));

                            if (w.enemyType == EnemyType::NAZGUL) {
                                isBossActive = true;
                                bossLabelTimer = 4.0f;
                            }

                            int rndSpawn = GetRandomValue(1, 3);
                            Audio::PlaySFX(TextFormat("spawn_%d", rndSpawn), 0.1f + GetRandomValue(-1, 1) / 10.0f);
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
                if (enemies.empty()) {
                    Audio::StopMusic();
                    Audio::PlayMusic("victory_jingle");
                    currentScreen = GameScreen::VICTORY;
                }
            }

            if (!enemies.empty()) {
                walkSoundTimer += dt;
                if (walkSoundTimer > 0.6f) {
                    walkSoundTimer = 0.0f;
                    bool hasInfantry = false;
                    bool hasHeavy = false;
                    bool hasNazgul = false;
                    for (const auto& e : enemies) {
                        if (e.GetType() == EnemyType::ORC || e.GetType() == EnemyType::URUK) hasInfantry = true;
                        if (e.GetType() == EnemyType::TROLL || e.GetType() == EnemyType::GROND || e.GetType() == EnemyType::COMMANDER) hasHeavy = true;
                        if (e.GetType() == EnemyType::NAZGUL) hasNazgul = true;
                    }
                    if (hasInfantry) Audio::PlaySFX("orc_walk", 0.3f, 1.0f);
                    if (hasHeavy) Audio::PlaySFX("heavy_walk", 0.2f, 0.8f);
                    if (hasNazgul) Audio::PlaySFX("heavy_walk", 0.7f, 0.1f);
                }
            }

            bloodSystem.Update(dt);
            for (int i = 0; i < enemies.size(); i++) {
                enemies[i].Update(dt);
                if (!enemies[i].IsAlive()) {
                    gold += 15;
                    urukBlood += enemies[i].GetManaReward() * 0.4f;
                    if (urukBlood > MAX_BLOOD) urukBlood = MAX_BLOOD;
                    Audio::PlaySFX("orc_death", 0.1f, GetRandomValue(80, 120) / 100.0f);
                    Audio::PlaySFX("gold_gain", 0.1f, 1.0f + GetRandomValue(0, 2) / 10.0f);
                    enemies.erase(enemies.begin() + i); i--; continue;
                }
                if (enemies[i].ReachedEnd()) {
                    castleHealth -= enemies[i].GetDamage();
                    enemies.erase(enemies.begin() + i); i--;
                    if (castleHealth <= 0) {
                        castleHealth = 0;
                        Audio::StopMusic();
                        Audio::PlayMusic("game_over");
                        currentScreen = GameScreen::GAMEOVER;
                    }
                }
            }
            for (int i = 0; i < (int)riders.size(); i++) {
                riders[i].Update(dt);
                for (Enemy& e : enemies) {
                    if (e.IsAlive() && CheckCollisionCircles(riders[i].position, 30, e.GetPosition(), 20)) {
                        float damageRate = 0.0f;
                        if (e.GetType() == EnemyType::NAZGUL || e.GetType() == EnemyType::TROLL) damageRate = 0.008f;
                        else damageRate = 0.02f;
                        int damage = (int)(e.GetHealth() * damageRate);
                        if (damage < 1) damage = 1;
                        e.TakeDamage(damage);
                        if (GetRandomValue(0, 4) == 0) bloodSystem.Spawn(e.GetPosition());
                        if (!e.IsAlive()) {
                            if (e.GetType() == EnemyType::NAZGUL || e.GetType() == EnemyType::TROLL) { gold += 50; urukBlood += 20; }
                            else { gold += 3; urukBlood += (int)(e.GetManaReward() * 0.12f); }
                            if (urukBlood > MAX_BLOOD) urukBlood = MAX_BLOOD;
                        }
                    }
                }
                if (!riders[i].active) { riders.erase(riders.begin() + i); i--; }
            }
            for (Tower& t : towers) t.Update(dt, enemies, projectiles);
            for (int i = 0; i < projectiles.size(); i++) {
                projectiles[i].Update(dt);
                if (projectiles[i].active && projectiles[i].type != ProjectileType::MELEE) {
                    for (Enemy& e : enemies) {
                        if (e.IsAlive() && CheckCollisionCircles(projectiles[i].position, 5, e.GetPosition(), e.GetRadius())) {
                            e.TakeDamage(projectiles[i].damage);
                            bloodSystem.Spawn(e.GetPosition());
                            if (projectiles[i].type == ProjectileType::ICE) Audio::PlaySFX("ice_hit", 0.3f, 1.0f);
                            else Audio::PlaySFX("arrow_hit", 0.1f + GetRandomValue(0, 2) / 10.0f);
                            if (projectiles[i].type == ProjectileType::ICE) e.ApplySlow(0.5f, 2.0f);
                            projectiles[i].active = false;
                            if (!e.IsAlive()) { gold += 15; urukBlood += e.GetManaReward(); if (urukBlood > MAX_BLOOD) urukBlood = MAX_BLOOD; }
                            break;
                        }
                    }
                }
                if (!projectiles[i].active) { projectiles.erase(projectiles.begin() + i); i--; }
            }

            BeginMode2D(camera);

           
            if (currentLevel->background.id > 0) {
               
                SetTextureWrap(currentLevel->background, TEXTURE_WRAP_REPEAT);

              
                Rectangle sourceRec = { 0, 0, (float)currentLevel->mapWidth, (float)currentLevel->background.height };

               
                Rectangle destRec = { 0, 0, (float)currentLevel->mapWidth, (float)gameScreenHeight };

                DrawTexturePro(currentLevel->background, sourceRec, destRec, { 0, 0 }, 0.0f, WHITE);
            }
            else {
                
                DrawRectangle(0, 0, currentLevel->mapWidth, gameScreenHeight, currentLevel->bgColor);
            }

            for (int y = 0; y < MAP_ROWS; y++) {
                for (int x = 0; x < currentLevel->cols; x++) {
                    Rectangle destRect = { (float)x * TILE_SIZE, (float)y * TILE_SIZE, (float)TILE_SIZE, (float)TILE_SIZE };
                    if (currentLevel->tileMap[y][x] != 0) {
                        if (texRoad.id > 0) DrawTexturePro(texRoad, { 0,0,(float)texRoad.width,(float)texRoad.height }, destRect, { 0,0 }, 0, WHITE);
                        else DrawRectangleRec(destRect, BROWN);
                    }
                }
            }
            if (texCity.id > 0) {
                DrawTexturePro(texCity, { 0, 0, (float)texCity.width, (float)texCity.height }, { currentLevel->castlePos.x, currentLevel->castlePos.y, (float)texCity.width * currentLevel->castleScale, (float)texCity.height * currentLevel->castleScale }, { 0, 0 }, 0.0f, WHITE);
            }

            float healthPct = (float)castleHealth / CASTLE_MAX_HEALTH;
            float barX = currentLevel->castlePos.x + 20;
            float barY = currentLevel->castlePos.y - 20;
            float barW = 200.0f;
            float barH = 20.0f;
            DrawRectangle(barX, barY, barW, barH, DARKGRAY);
            Color barColor = GREEN;
            if (healthPct < 0.5f) barColor = ORANGE;
            if (healthPct < 0.2f) barColor = RED;
            DrawRectangle(barX, barY, barW * healthPct, barH, barColor);
            DrawRectangleLines(barX, barY, barW, barH, BLACK);
            DrawText(TextFormat("%d / %d", castleHealth, CASTLE_MAX_HEALTH), barX + 60, barY + 2, 20, WHITE);

            for (const auto& t : towers) t.Draw();
            for (const auto& e : enemies) e.Draw();
            for (auto& r : riders) r.Draw();
            bloodSystem.Draw();
            for (const auto& p : projectiles) p.Draw();

            if (!isHoveringUI) {
                Texture2D previewTex = texTowerArcher;
                if (selectedTower == TowerType::MELEE) previewTex = texTowerMelee;
                else if (selectedTower == TowerType::ICE) previewTex = texTowerIce;
                Color ghostColor = isValidPlacement ? Fade(GREEN, 0.5f) : Fade(RED, 0.5f);
                float range = GetTowerRange(selectedTower);
                DrawCircleV(snapPos, range, Fade(ghostColor, 0.2f));
                DrawCircleLines((int)snapPos.x, (int)snapPos.y, range, ghostColor);
                if (previewTex.id > 0) {
                    Rectangle src = { 0, 0, (float)previewTex.width, (float)previewTex.height };
                    Rectangle dest = { snapPos.x, snapPos.y, 64, 114 };
                    Vector2 origin = { 32, 100 };
                    DrawTexturePro(previewTex, src, dest, origin, 0.0f, ghostColor);
                }
            }
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

            if (bossLabelTimer > 0.0f) {
                bossLabelTimer -= dt;
                float alpha = (sinf(GetTime() * 10.0f) + 1.0f) / 2.0f;
                const char* text = "WITCH KING IS HERE";
                int fontSize = 60;
                int textW = MeasureText(text, fontSize);
                int textX = (gameScreenWidth - textW) / 2;
                int textY = 200;
                DrawText(text, textX + 4, textY + 4, fontSize, Fade(BLACK, 0.7f));
                DrawText(text, textX, textY, fontSize, Fade(RED, 0.8f + (alpha * 0.2f)));
            }
            if (flashTimer > 0.0f) {
                flashTimer -= dt;
                DrawRectangle(0, 0, gameScreenWidth, gameScreenHeight, Fade(WHITE, flashTimer * 0.2f));
                float cellSize = 64.0f;
                int animFrame = (int)((2.0f - flashTimer) * 4.0f) % 7;
                Rectangle source = { animFrame * cellSize, 2 * cellSize, cellSize, cellSize };
                DrawTexturePro(texGandalf, source, { (float)gameScreenWidth / 2 - 64, (float)gameScreenHeight / 2 - 80, 128, 128 }, { 0,0 }, 0.0f, Fade(WHITE, flashTimer + 0.2f));
            }

            DrawRectangle(0, gameScreenHeight - 60, gameScreenWidth, 60, Fade(BLACK, 0.9f));
            DrawText(TextFormat("Gold: %d", gold), 20, gameScreenHeight - 45, 20, YELLOW);
            DrawText(TextFormat("Wave: %d / %d", currentWaveIndex + 1, currentLevel->waves.size()), 20, gameScreenHeight - 25, 20, WHITE);
            Color c1 = (selectedTower == TowerType::ARCHER) ? YELLOW : GRAY;
            Color c2 = (selectedTower == TowerType::MELEE) ? RED : GRAY;
            Color c3 = (selectedTower == TowerType::ICE) ? SKYBLUE : GRAY;
            DrawText("[1] Archer", 400, gameScreenHeight - 40, 20, c1);
            DrawText("[2] Melee", 550, gameScreenHeight - 40, 20, c2);
            DrawText("[3] Ice", 700, gameScreenHeight - 40, 20, c3);
            int rightX = gameScreenWidth - 450; int uiBarY = gameScreenHeight - 35;
            DrawRectangleLines(rightX, uiBarY, 120, 15, GRAY);
            float bloodPct = (float)urukBlood / MAX_BLOOD;
            DrawRectangle(rightX + 1, uiBarY + 1, (int)(118 * bloodPct), 13, MAROON);
            DrawText("BLOOD", rightX + 40, uiBarY + 3, 10, WHITE);
            Color cQ = (urukBlood >= COST_GANDALF) ? WHITE : DARKGRAY;
            Color cW = (urukBlood >= COST_ROHIRRIM) ? WHITE : DARKGRAY;
            DrawText("[Q] GANDALF", rightX + 130, uiBarY - 5, 20, cQ);
            DrawText("[W] ROHIRRIM", rightX + 280, uiBarY - 5, 20, cW);

           
            if (GuiButton({ (float)gameScreenWidth - 120, 10, 100, 30 }, "MENU", texBtnNormal, texBtnHover, mouseScreenPos)) {
                currentScreen = GameScreen::TITLE;
            }
        }
        break;

        case GameScreen::VICTORY:
            if (texVictoryBg.id > 0) DrawTexturePro(texVictoryBg, { 0, 0, (float)texVictoryBg.width, (float)texVictoryBg.height }, { 0, 0, (float)gameScreenWidth, (float)gameScreenHeight }, { 0, 0 }, 0.0f, WHITE);
            else DrawRectangle(0, 0, gameScreenWidth, gameScreenHeight, BLACK);
            DrawRectangle(0, 0, gameScreenWidth, gameScreenHeight, Fade(BLACK, 0.4f));
            DrawText("VICTORY!", gameScreenWidth / 2 - MeasureText("VICTORY!", 80) / 2, gameScreenHeight / 2 - 100, 80, GOLD);
            DrawText("Gondor is Safe... For now.", gameScreenWidth / 2 - MeasureText("Gondor is Safe... For now.", 30) / 2, gameScreenHeight / 2, 30, WHITE);
            // [FIX] mouseScreenPos parametresini ekledik
            if (GuiButton({ (float)gameScreenWidth / 2 - 100, (float)gameScreenHeight / 2 + 80, 200, 50 }, "MAIN MENU", texBtnNormal, texBtnHover, mouseScreenPos)) {
                Audio::StopMusic();
                currentScreen = GameScreen::TITLE;
            }
            break;

        case GameScreen::GAMEOVER:
            if (texDefeatBg.id > 0) DrawTexturePro(texDefeatBg, { 0, 0, (float)texDefeatBg.width, (float)texDefeatBg.height }, { 0, 0, (float)gameScreenWidth, (float)gameScreenHeight }, { 0, 0 }, 0.0f, WHITE);
            else DrawRectangle(0, 0, gameScreenWidth, gameScreenHeight, BLACK);
            DrawRectangle(0, 0, gameScreenWidth, gameScreenHeight, Fade(BLACK, 0.6f));
            DrawRectangle(0, 0, gameScreenWidth, gameScreenHeight, Fade(RED, 0.2f));
            DrawText("DEFEAT", gameScreenWidth / 2 - MeasureText("DEFEAT", 80) / 2, gameScreenHeight / 2 - 100, 80, RED);
            DrawText("The White City has Fallen.", gameScreenWidth / 2 - MeasureText("The White City has Fallen.", 30) / 2, gameScreenHeight / 2, 30, RAYWHITE);
            
            if (GuiButton({ (float)gameScreenWidth / 2 - 100, (float)gameScreenHeight / 2 + 80, 200, 50 }, "MAIN MENU", texBtnNormal, texBtnHover, mouseScreenPos)) {
                Audio::StopMusic();
                currentScreen = GameScreen::TITLE;
            }
            break;
        }

        EndTextureMode(); 
        BeginDrawing();
        ClearBackground(BLACK);

        Rectangle sourceRec = { 0.0f, 0.0f, (float)target.texture.width, -(float)target.texture.height };
        Rectangle destRec = { (GetScreenWidth() - (gameScreenWidth * scale)) * 0.5f, (GetScreenHeight() - (gameScreenHeight * scale)) * 0.5f, gameScreenWidth * scale, gameScreenHeight * scale };
        DrawTexturePro(target.texture, sourceRec, destRec, { 0, 0 }, 0.0f, WHITE);

        EndDrawing();
    } 

   
    UnloadTexture(texOrc); UnloadTexture(texUruk); UnloadTexture(texTroll); UnloadTexture(texGrond); UnloadTexture(texCommander);
    UnloadTexture(texTowerArcher); UnloadTexture(texTowerMelee); UnloadTexture(texTowerIce);
    UnloadTexture(texMenuBg); UnloadTexture(texBtnNormal); UnloadTexture(texBtnHover);
    UnloadTexture(texProjArrow); UnloadTexture(texProjIce); UnloadTexture(texProjMelee);
    UnloadTexture(texBlood); UnloadTexture(texRoad); UnloadTexture(texCity);
    UnloadTexture(texGandalf);
    UnloadTexture(texVictoryBg);
    UnloadTexture(texDefeatBg);
    UnloadRenderTexture(target);
    for (auto& lvl : allLevels) {
        UnloadTexture(lvl.background);
        for (auto* p : lvl.paths) delete p;
    }

    Audio::Close();
    CloseWindow();
    return 0;
}
