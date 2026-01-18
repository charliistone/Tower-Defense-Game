#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>

enum class EnemyType { ORC, URUK, TROLL };

class Enemy {
public:
    // ZORLUK SİSTEMİ: speedMult (Hız Çarpanı) ve hpBonus (Ekstra Can) eklendi
    Enemy(EnemyType type, std::vector<Vector2>* path, Texture2D tex, float speedMult = 1.0f, int hpBonus = 0);

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

    // Animasyon Değişkenleri
    int frameWidth;
    int frameHeight;
    int currentFrame;
    float animTimer;
    int facing;
};