#pragma once
#include "raylib.h"

enum class ProjectileType {
    NORMAL, // Standard Damage
    ICE     // Low Damage + Slow Effect
};

struct Projectile {
    Vector2 position;
    Vector2 velocity;
    int damage;
    bool active;
    ProjectileType type; // <--- NEW: Remembers if it is Ice or Normal

    Projectile(Vector2 pos, Vector2 vel, int dmg, ProjectileType t)
        : position(pos), velocity(vel), damage(dmg), active(true), type(t) {
    }

    void Update(float dt) {
        position.x += velocity.x * dt;
        position.y += velocity.y * dt;

        // Deactivate if off-screen (simple optimization)
        if (position.x < 0 || position.x > 3000 || position.y < 0 || position.y > 3000) {
            active = false;
        }
    }

    void Draw() const {
        // Draw Ice balls Blue, Arrows Yellow
        Color c = (type == ProjectileType::ICE) ? SKYBLUE : YELLOW;
        DrawCircleV(position, 5, c);
    }
};