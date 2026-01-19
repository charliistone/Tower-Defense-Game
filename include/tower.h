#pragma once
#include "raylib.h"
#include "Enemy.h"
#include "Projectile.h"
#include <vector>

enum class TowerType { ARCHER, MELEE, ICE };

class Tower {
public:
    
    Tower(Vector2 pos, Texture2D tex, Texture2D projTex, TowerType type);

    void Update(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles);
    void Draw() const;
    void Upgrade();

    bool IsClicked(Vector2 mousePos) const;
    Vector2 GetPosition() const { return position; }
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
