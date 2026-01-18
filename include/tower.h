#pragma once
#include "raylib.h"
#include "raymath.h"
#include "Enemy.h"
#include "Projectile.h"
#include <vector>

// Kule Tipleri
enum class TowerType { ARCHER, MELEE, ICE };

class Tower {
public:
    Tower(Vector2 pos, Texture2D tex, TowerType type);

    void Update(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles);
    void Draw() const;

    // Etkileşim Fonksiyonları
    bool IsClicked(Vector2 mousePos) const;
    void Upgrade();

    // Getterlar
    int GetUpgradeCost() const;
    int GetLevel() const { return level; } // YENİ: Level bilgisini döndürür
    float GetRange() const { return range; }
    Vector2 GetPosition() const { return position; }

private:
    Vector2 position;
    Texture2D texture;
    TowerType type;

    // İstatistikler
    int level;      // YENİ: Kule seviyesi
    float range;
    float damage;
    float cooldown;
    float timer;
    int cost;
};