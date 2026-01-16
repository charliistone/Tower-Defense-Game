#pragma once
#include "raylib.h"
#include "raymath.h"
#include "Enemy.h"
#include "Projectile.h" // <--- ADD THIS INCLUDE
#include <vector>

class Tower {
public:
    Tower(Vector2 pos, Texture2D tex);

    // Update needs to know about the projectile list to add new bullets
    void Update(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles);

    void Draw() const;

private:
    Vector2 position;
    Texture2D texture;
    float range;
    float fireRate;
    float cooldown;
    int damage;
};