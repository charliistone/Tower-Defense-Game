#include "Enemy.h"
#include "raymath.h"

Enemy::Enemy(EnemyType type, std::vector<Vector2>* path, Texture2D texture)
    : type(type), pathPoints(path), texture(texture),
    currentPathIndex(0), alive(true), reachedEnd(false),
    facingDirection(0) // Default face Down
{
    InitStatsByType();
    position = (*pathPoints)[0];
}

void Enemy::InitStatsByType() {
    switch (type) {
    case EnemyType::ORC:
        maxHp = 30;
        baseSpeed = 100.0f; // Pixels per second
        radius = 40.0f;
        break;
    case EnemyType::URUK:
        maxHp = 80;
        baseSpeed = 80.0f;
        radius = 45.0f;
        break;
    case EnemyType::TROLL:
        maxHp = 500;
        baseSpeed = 40.0f;
        radius = 60.0f;
        break;
    }
    hp = maxHp;
    speed = baseSpeed; // Initialize current speed
}

void Enemy::ApplySlow(float value, float duration) {
    // Add the slow effect to the list
    effects.push_back({ StatusEffectType::SLOW, value, duration });
}

void Enemy::TakeDamage(int dmg) {
    hp -= dmg;
    if (hp <= 0) {
        alive = false;
    }
}

void Enemy::Update(float dt) {
    if (!alive) return;

    // --- STEP 1: HANDLE STATUS EFFECTS ---
    speed = baseSpeed; // Reset to normal speed first

    for (int i = effects.size() - 1; i >= 0; i--) {
        effects[i].duration -= dt;

        // Apply the slow
        if (effects[i].type == StatusEffectType::SLOW) {
            speed *= effects[i].value;
        }

        // Remove if time is up
        if (effects[i].duration <= 0) {
            effects.erase(effects.begin() + i);
        }
    }

    // --- STEP 2: MOVEMENT ---
    if (currentPathIndex < pathPoints->size() - 1) {
        Vector2 target = (*pathPoints)[currentPathIndex + 1];
        Vector2 dir = Vector2Subtract(target, position);
        float dist = Vector2Length(dir);

        // Normalize direction
        dir = Vector2Normalize(dir);

        // --- STEP 3: ANIMATION DIRECTION ---
        // Determine which way we are looking based on movement vector
        if (fabs(dir.x) > fabs(dir.y)) {
            // Horizontal
            if (dir.x > 0) facingDirection = 2; // Right
            else facingDirection = 1;           // Left
        }
        else {
            // Vertical
            if (dir.y > 0) facingDirection = 0; // Down
            else facingDirection = 3;           // Up
        }

        // Move using the CALCULATED speed (not baseSpeed)
        float moveAmount = speed * dt;

        if (dist <= moveAmount) {
            position = target;
            currentPathIndex++;
            if (currentPathIndex >= pathPoints->size() - 1) {
                reachedEnd = true;
                alive = false;
            }
        }
        else {
            position = Vector2Add(position, Vector2Scale(dir, moveAmount));
        }
    }
}

void Enemy::Draw() const {
    if (!alive) return;

    // --- SPRITE SLICING ---
    float columns = 4.0f;
    float rows = 4.0f;

    float frameWidth = texture.width / columns;
    float frameHeight = texture.height / rows;

    // PICK THE ROW BASED ON FACING DIRECTION
    // 0=Down, 1=Left, 2=Right, 3=Up
    float currentY = facingDirection * frameHeight;

    Rectangle sourceRec = { 0.0f, currentY, frameWidth, frameHeight };
    Rectangle destRec = { position.x, position.y, frameWidth, frameHeight };
    Vector2 origin = { frameWidth / 2.0f, frameHeight / 2.0f };

    DrawTexturePro(texture, sourceRec, destRec, origin, 0.0f, WHITE);

    // Draw HP Bar
    float hpPercent = (float)hp / maxHp;
    DrawRectangle(position.x - 15, position.y - 30, 30, 5, RED);
    DrawRectangle(position.x - 15, position.y - 30, 30 * hpPercent, 5, GREEN);
}