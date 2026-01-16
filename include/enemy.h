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
    float value;     // e.g. 0.5 for 50% slow
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
    void ApplySlow(float value, float duration); // <-- Function to apply slow

    bool IsAlive() const { return alive; }
    bool ReachedEnd() const { return reachedEnd; }

    Vector2 GetPosition() const { return position; }
    float GetRadius() const { return radius; }
    EnemyType GetType() const { return type; }

private:
    void InitStatsByType();

    EnemyType type;
    Vector2 position;

    // MOVEMENT VARIABLES
    int facingDirection; // 0=Down, 1=Left, 2=Right, 3=Up (NEW)
    float speed;         // Current speed (affected by slows)
    float baseSpeed;     // Original speed (max speed)

    // STATS
    int hp;
    int maxHp;
    float radius;

    // PATHFINDING
    std::vector<Vector2>* pathPoints;
    int currentPathIndex;

    // STATE
    bool alive;
    bool reachedEnd;

    // VISUALS
    Texture2D texture;

    // EFFECTS LIST
    std::vector<StatusEffect> effects; // List of active effects
};