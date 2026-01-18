#include "Enemy.h"
#include "raylib.h"

extern Sound deathSound;

Enemy::Enemy(EnemyType type, std::vector<Vector2>* path, Texture2D tex)
    : position({ 0,0 }), path(path), currentPoint(0), texture(tex), type(type),
    alive(true), health(0), maxHealth(0), speed(0.0f), distanceTraveled(0.0f),
    manaReward(0), stunTimer(0.0f), slowTimer(0.0f), slowFactor(1.0f), frozen(false),
    currentFrame(0), animTimer(0.0f), facing(0)
{
    if (path && !path->empty()) position = (*path)[0];

    // Stats
    if (type == EnemyType::ORC) {
        maxHealth = 10; speed = 100.0f; manaReward = 2;
    }
    else if (type == EnemyType::URUK) {
        maxHealth = 30; speed = 80.0f; manaReward = 5;
    }
    else if (type == EnemyType::TROLL) {
        maxHealth = 150; speed = 40.0f; manaReward = 20;
    }
    health = maxHealth;

    // --- SPRITE HESABI ---
    // Spreadsheet 3 sütun (hareket) ve 4 satýr (yön) varsayýlýyor.
    frameWidth = texture.width / 3;
    frameHeight = texture.height / 4;

    // Eðer görsel kare ise (Spreadsheet deðilse) düzelt
    if (texture.width == texture.height) {
        frameWidth = texture.width;
        frameHeight = texture.height;
    }
}

void Enemy::Update(float dt) {
    if (!alive) return;

    if (stunTimer > 0.0f) { stunTimer -= dt; return; }

    float currentSpeed = speed;
    if (slowTimer > 0.0f) {
        slowTimer -= dt;
        currentSpeed = speed * slowFactor;
        frozen = true;
    }
    else {
        frozen = false;
        slowFactor = 1.0f;
    }

    if (path && currentPoint < path->size() - 1) {
        Vector2 target = (*path)[currentPoint + 1];
        Vector2 direction = Vector2Normalize(Vector2Subtract(target, position));

        // --- YÖN HESAPLAMA (MOONWALK FIX) ---
        if (fabs(direction.x) > fabs(direction.y)) {
            // Yatay Hareket
            if (direction.x > 0) facing = 2; // Sað (Genelde Row 2)
            else facing = 1;                 // Sol  (Genelde Row 1)
        }
        else {
            // Dikey Hareket
            if (direction.y > 0) facing = 0; // Aþaðý (Genelde Row 0)
            else facing = 3;                 // Yukarý (Genelde Row 3)
        }

        // --- ADIM ANIMASYONU ---
        animTimer += dt;
        if (animTimer >= 0.2f) { // Adým hýzý
            animTimer = 0.0f;
            currentFrame++;
            if (currentFrame > 2) currentFrame = 0; // 0-1-2 döngüsü
        }

        position = Vector2Add(position, Vector2Scale(direction, currentSpeed * dt));

        if (Vector2Distance(position, target) < 5.0f) currentPoint++;
    }
}

void Enemy::Draw() const {
    if (!alive) return;

    float drawSize = 48.0f;
    if (type == EnemyType::TROLL) drawSize = 64.0f;

    // --- DOÐRU KAREYÝ KESME (CROP) ---
    // Eðer görsel spreadsheet deðilse (örn: kare ise), kaynak tüm resimdir.
    Rectangle source;
    if (texture.width == texture.height) {
        source = { 0, 0, (float)texture.width, (float)texture.height };
    }
    else {
        // Spreadsheet ise doðru parçayý al
        source = {
            (float)currentFrame * frameWidth,
            (float)facing * frameHeight,
            (float)frameWidth,
            (float)frameHeight
        };
    }

    Rectangle dest = { position.x, position.y, drawSize, drawSize };
    Vector2 origin = { drawSize / 2.0f, drawSize / 2.0f };

    Color tint = WHITE;
    if (stunTimer > 0.0f) tint = GOLD;
    else if (frozen) tint = SKYBLUE;

    DrawTexturePro(texture, source, dest, origin, 0.0f, tint);

    // Can Barý
    float pct = (float)health / (float)maxHealth;
    int barWidth = (int)drawSize;
    DrawRectangle((int)position.x - barWidth / 2, (int)position.y - (int)(drawSize / 2) - 8, barWidth, 5, RED);
    DrawRectangle((int)position.x - barWidth / 2, (int)position.y - (int)(drawSize / 2) - 8, (int)(barWidth * pct), 5, GREEN);
}

void Enemy::TakeDamage(int dmg) {
    if (!alive) return;

    health -= dmg;
    if (health <= 0) {
        alive = false;
        PlaySound(deathSound);
    }
}

void Enemy::ApplyStun(float duration) { stunTimer = duration; }
void Enemy::ApplySlow(float factor, float duration) { slowFactor = factor; slowTimer = duration; }