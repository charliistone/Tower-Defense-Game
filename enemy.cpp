#include "Enemy.h"
#include <cmath>

Enemy::Enemy(EnemyType type,
             std::vector<Vector2>* path,
             Texture2D texture)
    : type(type),
      pathPoints(path),
      texture(texture),
      currentPathIndex(0),
      alive(true),
      reachedEnd(false)
{
    InitStatsByType();
    position = (*pathPoints)[0];
}

void Enemy::InitStatsByType() {
    switch (type) {
    case EnemyType::ORC:
        baseSpeed = 90.0f;
        maxHp = 60;
        radius = 16.0f;
        break;

    case EnemyType::URUK:
        baseSpeed = 60.0f;
        maxHp = 120;
        radius = 18.0f;
        break;

    case EnemyType::TROLL:
        baseSpeed = 35.0f;
        maxHp = 300;
        radius = 26.0f;
        break;
    }

    speed = baseSpeed;
    hp = maxHp;
}

void Enemy::Update(float dt) {
    if (!alive || reachedEnd) return;

    speed = baseSpeed;

    for (auto it = effects.begin(); it != effects.end();) {
        it->duration -= dt;

        if (it->type == StatusEffectType::SLOW)
            speed *= (1.0f - it->value);

        if (it->duration <= 0)
            it = effects.erase(it);
        else
            ++it;
    }

    if (currentPathIndex >= pathPoints->size()) {
        reachedEnd = true;
        alive = false;
        return;
    }

    Vector2 target = (*pathPoints)[currentPathIndex];
    Vector2 dir = {
        target.x - position.x,
        target.y - position.y
    };

    float len = sqrtf(dir.x * dir.x + dir.y * dir.y);

    if (len < 2.0f) {
        currentPathIndex++;
        return;
    }

    dir.x /= len;
    dir.y /= len;

    position.x += dir.x * speed * dt;
    position.y += dir.y * speed * dt;
}

void Enemy::Draw() const {
    if (!alive) return;

    DrawTexturePro(
        texture,
        { 0, 0, (float)texture.width, (float)texture.height },
        { position.x, position.y, radius * 2, radius * 2 },
        { radius, radius },
        0.0f,
        WHITE
    );

    // HP bar
    float hpRatio = (float)hp / maxHp;
    DrawRectangle(position.x - 20, position.y - radius - 10, 40, 5, RED);
    DrawRectangle(position.x - 20, position.y - radius - 10, 40 * hpRatio, 5, GREEN);
}

void Enemy::TakeDamage(int dmg) {
    hp -= dmg;
    if (hp <= 0) {
        alive = false;
    }
}

void Enemy::ApplySlow(float value, float duration) {
    effects.push_back({ StatusEffectType::SLOW, value, duration });
}

bool Enemy::IsAlive() const {
    return alive;
}

bool Enemy::ReachedEnd() const {
    return reachedEnd;
}

Vector2 Enemy::GetPosition() const {
    return position;
}

float Enemy::GetRadius() const {
    return radius;
}

EnemyType Enemy::GetType() const {
    return type;
}
