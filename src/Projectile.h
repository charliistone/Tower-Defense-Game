#pragma once
#include "raylib.h"
#include "raymath.h"

// Mermi Tipleri
enum class ProjectileType { ARROW, ICE, BOMB };

struct Projectile {
    Vector2 position;
    Vector2 target;
    Vector2 velocity;
    int damage;
    ProjectileType type;
    bool active;

    // Constructor - Tower.cpp bu yapıyı kullanıyor
    Projectile(Vector2 pos, Vector2 tar, int dmg, ProjectileType t) {
        position = pos;
        target = tar;
        damage = dmg;
        type = t;
        active = true;

        // Hedefe yönelme
        Vector2 dir = Vector2Normalize(Vector2Subtract(target, position));
        float speed = 600.0f; // Mermi hızı
        velocity = Vector2Scale(dir, speed);
    }

    void Update(float dt) {
        position = Vector2Add(position, Vector2Scale(velocity, dt));

        // Ekran dışına çıkarsa sil
        if (position.x < -50 || position.x > 1330 || position.y < -50 || position.y > 770) {
            active = false;
        }
    }

    void Draw() const {
        Color c = BLACK;
        if (type == ProjectileType::ICE) c = SKYBLUE;
        else if (type == ProjectileType::BOMB) c = RED;

        DrawCircleV(position, 5, c);
    }
};