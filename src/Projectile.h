#pragma once
#include "raylib.h"
#include "raymath.h"

enum class ProjectileType {
    ARROW,
    ICE
};

class Projectile {
public:
    Projectile(Vector2 start, Vector2 target, int dmg, ProjectileType t) {
        position = start;
        damage = dmg;
        type = t;
        active = true;
        speed = 600.0f; 

        Vector2 dir = Vector2Normalize(Vector2Subtract(target, start));
        velocity = Vector2Scale(dir, speed);
    }

    void Update(float dt) {
        position = Vector2Add(position, Vector2Scale(velocity, dt));

        
        if (position.x < 0 || position.x > 3000 || position.y < 0 || position.y > 3000) {
            active = false;
        }
    }

    void Draw() const {
        if (!active) return;
        Color c = (type == ProjectileType::ICE) ? SKYBLUE : BLACK;
        DrawCircleV(position, 5, c);
    }

    Vector2 position;
    bool active;
    int damage;
    ProjectileType type;

private:
    Vector2 velocity;
    float speed;
};
