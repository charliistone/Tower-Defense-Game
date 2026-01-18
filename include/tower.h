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
    // DÜZELTME: Constructor artık 4 parametre alıyor (Mermi resmi eklendi)
    Tower(Vector2 pos, Texture2D tex, Texture2D projTex, TowerType type);

    void Update(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles);
    void Draw() const;
    void Upgrade();

    int GetUpgradeCost() const;
    int GetLevel() const { return level; }
    int GetCost() const { return cost; }
    bool IsClicked(Vector2 mousePos) const;
    float GetRange() const { return range; }
    Vector2 GetPosition() const { return position; }

private:
    Vector2 position;
    Texture2D texture;
    Texture2D projTexture; // <-- YENİ: Mermi resmi için değişken
    TowerType type;

    int level;
    float range;
    int damage;
    float fireRate;
    float cooldown;
    int cost;
};