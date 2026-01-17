#pragma once
#include "raylib.h"
#include "Enemy.h"
#include "Projectile.h"
#include <vector>

enum class TowerType {
    ARCHER,
    ICE,
    MELEE
};

class Tower {
public:
    Tower(Vector2 pos, Texture2D tex, TowerType type);

    void Update(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles);
    void Draw() const;

    // --- NEW UPGRADE SYSTEM ---
    void Upgrade();             // Increases stats
    int GetUpgradeCost() const; // Calculates price
    int GetLevel() const { return level; }

    // Check if mouse is touching this tower
    bool IsClicked(Vector2 mousePos) const;

    // Getters for main.cpp to check logic
    int GetCost() const { return cost; }

private:
    Vector2 position;
    Texture2D texture;

    TowerType type;
    int level;      // <--- NEW: Starts at 1
    float range;
    int damage;
    float fireRate;
    float cooldown;
    int cost;       // Base cost
};