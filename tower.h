#pragma once
#include "raylib.h"
#include <vector>
#include "Enemy.h"
#include "Projectile.h"

class Tower {
public:
    Tower(Vector2 pos, Texture2D tex);

    virtual void Update(float dt,
        std::vector<Enemy>& enemies,
        std::vector<Projectile>& projectiles);

    virtual void Draw() const;

protected:
    Vector2 position;
    float range;
    float fireRate;
    float cooldown;
    int damage;
    Texture2D texture;
};
