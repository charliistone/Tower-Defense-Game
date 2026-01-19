#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>

// An enum-type class that defines enemy variations within the game.
enum class EnemyType { ORC, URUK, TROLL, GROND, COMMANDER, NAZGUL };

class Enemy {
public:
    
    Enemy(EnemyType type, std::vector<Vector2>* path, Texture2D tex, float speedMult = 1.0f, int hpBonus = 0);

    void Update(float dt);
    void Draw() const;

    void TakeDamage(int dmg);

    // Status Effects: Managing mechanics that restrict the enemy's movement.
    void ApplyStun(float duration);
    void ApplySlow(float factor, float duration);

    bool IsAlive() const { return alive; }
    bool ReachedEnd() const { return currentPoint >= path->size() - 1; }
    Vector2 GetPosition() const { return position; }

    // Radius used for collision and turret range control. Returns 'Hitbox' size customized according to enemy type.
    float GetRadius() const {
        if (type == EnemyType::GROND) return 60.0f;
        if (type == EnemyType::NAZGUL) return 40.0f; 
        if (type == EnemyType::TROLL) return 30.0f;
        return 15.0f;
    }
    int GetManaReward() const { return manaReward; }
    int GetDamage() const { return damage; }
    int GetHealth() const { return health; }
    EnemyType GetType() const { return type; }

private:
    Vector2 position;
    std::vector<Vector2>* path;    // Reference to the vector holding the path coordinates.
    int currentPoint;    // The coordinates of the enemy's current advance.
    Texture2D texture;
    EnemyType type;

    bool alive;
    int health;
    int maxHealth;
    float speed;
    float distanceTraveled;

    int manaReward;
    float stunTimer;
    float slowTimer;
    float slowFactor;
    bool frozen;
    int damage;

    int frameWidth;
    int frameHeight;
    int currentFrame;
    float animTimer;
    int facing;
};
