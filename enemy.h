#pragma once
#include "raylib.h"
#include <vector>

// Enemy types
enum class EnemyType {
    ORC,
    URUK,
    TROLL
};

// Status effect types
enum class StatusEffectType {
    SLOW
};

// Status effect struct
struct StatusEffect {
    StatusEffectType type;
    float value;     // e.g. 0.4 = %40 slow
    float duration;  // seconds
};

class Enemy {
public:
    Enemy(EnemyType type,
          std::vector<Vector2>* path,
          Texture2D texture);

    void Update(float dt);
    void Draw() const;

    void TakeDamage(int dmg);
    void ApplySlow(float value, float duration);

    bool IsAlive() const;
    bool ReachedEnd() const;

    Vector2 GetPosition() const;
    float GetRadius() const;
    EnemyType GetType() const;

private:
    void InitStatsByType();

    EnemyType type;

    Vector2 position;

    float speed;       // current speed (after effects)
    float baseSpeed;   // original speed (no effects)

    int hp;
    int maxHp;
    float radius;

    std::vector<Vector2>* pathPoints;
    int currentPathIndex;

    bool alive;
    bool reachedEnd;

    Texture2D texture;

    std::vector<StatusEffect> effects;
};
