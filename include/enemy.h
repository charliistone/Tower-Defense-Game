#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>

// DÝKKAT: Buraya ASLA #include "Enemy.h" EKLEME! Sonsuz döngü yapar.

enum class EnemyType { ORC, URUK, TROLL };

class Enemy {
public:
    Enemy(EnemyType type, std::vector<Vector2>* path, Texture2D tex);

    void Update(float dt);
    void Draw() const;

    void TakeDamage(int dmg);
    void ApplyStun(float duration);
    void ApplySlow(float factor, float duration);

    bool IsAlive() const { return alive; }
    bool ReachedEnd() const { return currentPoint >= path->size() - 1; }
    Vector2 GetPosition() const { return position; }
    float GetRadius() const { return 15.0f; }
    int GetManaReward() const { return manaReward; }

private:
    Vector2 position;
    std::vector<Vector2>* path;
    int currentPoint;
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

    // --- ANIMASYON DEÐÝÞKENLERÝ ---
    int frameWidth;
    int frameHeight;
    int currentFrame;   // Hangi adým karesi? (0, 1, 2)
    float animTimer;    // Animasyon hýzý
    int facing;         // Hangi yöne bakýyor? (0:Aþaðý, 1:Sol, 2:Sað, 3:Yukarý)
};