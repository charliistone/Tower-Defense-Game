#pragma once
#pragma once
#include "raylib.h"

struct Projectile {
    Vector2 position;
    Vector2 velocity;
    int damage;
    bool active; // Is it still flying?

    // Constructor
    Projectile(Vector2 pos, Vector2 vel, int dmg);

    // Functions
    void Update(float dt);
    void Draw() const;
};