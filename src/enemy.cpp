#include "Enemy.h"
#include "raylib.h"

extern Sound deathSound;

Enemy::Enemy(EnemyType type, std::vector<Vector2>* path, Texture2D tex)
    : position({ 0,0 }), path(path), currentPoint(0), texture(tex), type(type),
    alive(true), health(0), maxHealth(0), speed(0.0f), distanceTraveled(0.0f),
    manaReward(0), goldReward(0), stunTimer(0.0f), slowTimer(0.0f), slowFactor(1.0f), frozen(false),
    currentFrame(0), animTimer(0.0f), facing(0)
{
    if (path && !path->empty()) position = (*path)[0];

    // --- ZORLUK AYARLARI ---
    if (type == EnemyType::ORC) {
        maxHealth = 30;   // Daha güçlü (Eski: 10)
        speed = 110.0f;
        manaReward = 4;
        goldReward = 6;   // Az para (Eski: 15)
    }
    else if (type == EnemyType::URUK) {
        maxHealth = 70;  // Tank (Eski: 30)
        speed = 90.0f;
        manaReward = 7;
        goldReward = 12;
    }
    else if (type == EnemyType::TROLL) {
        maxHealth = 600; // BOSS (Eski: 150)
        speed = 55.0f;
        manaReward = 20;
        goldReward = 40;
    }
    health = maxHealth;

    // Sprite ayarları
    frameWidth = texture.width / 3;
    frameHeight = texture.height / 4;
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

        // Yön
        if (fabs(direction.x) > fabs(direction.y)) {
            if (direction.x > 0) facing = 2; else facing = 1;
        }
        else {
            if (direction.y > 0) facing = 0; else facing = 3;
        }

        // Animasyon
        animTimer += dt;
        if (animTimer >= 0.2f) {
            animTimer = 0.0f;
            currentFrame++;
            if (currentFrame > 2) currentFrame = 0;
        }

        position = Vector2Add(position, Vector2Scale(direction, currentSpeed * dt));
        if (Vector2Distance(position, target) < 5.0f) currentPoint++;
    }
}

void Enemy::Draw() const {
    if (!alive) return;

    float drawSize = 48.0f;
    if (type == EnemyType::TROLL) drawSize = 64.0f;

    Rectangle source;
    if (texture.width == texture.height) {
        source = { 0, 0, (float)texture.width, (float)texture.height };
    }
    else {
        source = { (float)currentFrame * frameWidth, (float)facing * frameHeight, (float)frameWidth, (float)frameHeight };
    }

    Rectangle dest = { position.x, position.y, drawSize, drawSize };
    Vector2 origin = { drawSize / 2.0f, drawSize / 2.0f };
    Color tint = (stunTimer > 0.0f) ? GOLD : (frozen ? SKYBLUE : WHITE);

    DrawTexturePro(texture, source, dest, origin, 0.0f, tint);

    // Can barı
    float pct = (float)health / (float)maxHealth;
    DrawRectangle((int)position.x - 20, (int)position.y - 30, 40, 5, RED);
    DrawRectangle((int)position.x - 20, (int)position.y - 30, (int)(40 * pct), 5, GREEN);
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