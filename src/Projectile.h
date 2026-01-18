#pragma once
#include "raylib.h"
#include "raymath.h"

enum class ProjectileType {
    ARROW,
    ICE
};

class Projectile {
public:
    // DÜZELTME: Constructor Texture2D alıyor
    Projectile(Vector2 start, Vector2 target, int dmg, ProjectileType t, Texture2D tex) {
        position = start;
        damage = dmg;
        type = t;
        texture = tex; // Kaydet
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

        // Sprite çizimi
        // Basitçe tüm resmi çiziyoruz, eğer sprite sheet kullanıyorsan source rectangle ayarlaman gerekir.
        // Şimdilik ortalayarak çizelim:
        if (texture.id > 0) {
            float rotation = atan2(velocity.y, velocity.x) * RAD2DEG;
            Rectangle source = { 0, 0, (float)texture.width, (float)texture.height };
            Rectangle dest = { position.x, position.y, (float)texture.width, (float)texture.height };
            Vector2 origin = { (float)texture.width / 2, (float)texture.height / 2 };
            DrawTexturePro(texture, source, dest, origin, rotation, WHITE);
        }
        else {
            // Texture yoksa yuvarlak çiz
            Color c = (type == ProjectileType::ICE) ? SKYBLUE : BLACK;
            DrawCircleV(position, 5, c);
        }
    }

    Vector2 position;
    bool active;
    int damage;
    ProjectileType type;

private:
    Vector2 velocity;
    float speed;
    Texture2D texture; // <-- Texture değişkeni
};