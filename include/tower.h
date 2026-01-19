#pragma once
#include "raylib.h"
#include "Enemy.h"
#include "Projectile.h"
#include <vector>

// Tower types: An enum-type class that specifies different damage, range, and special effects (slowness, etc.) for each type.
enum class TowerType { ARCHER, MELEE, ICE };

class Tower {
public:

    // When building the tower, importing the bullet texture (projTex) from an external source allows for efficient memory usage instead of reloading the texture with each shot.
    Tower(Vector2 pos, Texture2D tex, Texture2D projTex, TowerType type);

    void Update(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles);
    void Draw() const;
    void Upgrade();

    bool IsClicked(Vector2 mousePos) const;
    Vector2 GetPosition() const { return position; }
    // A simple formula where the cost increases exponentially as the level rises.
    int GetUpgradeCost() const { return cost * 2; }
    float GetRange() const { return range; }

private:
    Vector2 position;
    Texture2D texture;
    Texture2D projTexture; 
    TowerType type;

    int level;
    float cooldown;
    float range;
    int damage;
    float fireRate;
    int cost;
};
