#pragma once
#include "raylib.h"
#include "raymath.h"

enum class ProjectileType {
    ARROW,
    ICE,
    MELEE
};

class Projectile {
public:
    // YENİ: En sona 'float sc' (scale) parametresi ekledik. Varsayılan = 1.0f (Normal Boyut)
    Projectile(Vector2 start, Vector2 target, int dmg, ProjectileType t, Texture2D tex, float sc = 1.0f)
        : position(start), damage(dmg), type(t), texture(tex),
        active(true), currentFrame(0), animTimer(0.0f), scale(sc) // <-- Scale'i kaydet
    {
        // MELEE İÇİN ÖZEL AYAR
        if (type == ProjectileType::MELEE) {
            speed = 0.0f;
            velocity = { 0, 0 };
            rotation = 0.0f;
            position = target;
        }
        else {
            speed = 500.0f;
            Vector2 dir = Vector2Normalize(Vector2Subtract(target, start));
            velocity = Vector2Scale(dir, speed);
            rotation = atan2(dir.y, dir.x) * RAD2DEG;
        }

        // Animasyon Hesabı (6 Kare)
        frameWidth = texture.width / 6;
        frameHeight = texture.height;
    }

    void Update(float dt) {
        if (type != ProjectileType::MELEE) {
            position = Vector2Add(position, Vector2Scale(velocity, dt));
            if (position.x < -100 || position.x > 5000 || position.y < -100 || position.y > 5000) active = false;
        }

        animTimer += dt;
        if (animTimer >= 0.05f) {
            animTimer = 0.0f;
            currentFrame++;

            if (type == ProjectileType::MELEE && currentFrame >= 6) {
                active = false;
            }
            else if (currentFrame >= 6) {
                currentFrame = 0;
            }
        }
    }

    void Draw() const {
        if (!active) return;

        Rectangle source = { (float)currentFrame * frameWidth, 0, (float)frameWidth, (float)frameHeight };

        // --- BOYUT AYARI BURADA YAPILIYOR ---
        // Orijinal genişliği 'scale' ile çarpıyoruz.
        float destW = frameWidth * scale;
        float destH = frameHeight * scale;

        // Hedef dikdörtgen (Destination Rectangle) artık yeni boyutlara sahip
        Rectangle dest = { position.x, position.y, destW, destH };

        // Dönme merkezini de yeni boyuta göre tam ortaya alıyoruz
        Vector2 origin = { destW / 2, destH / 2 };

        DrawTexturePro(texture, source, dest, origin, rotation, WHITE);
    }

    Vector2 position;
    bool active;
    int damage;
    ProjectileType type;

private:
    Vector2 velocity;
    float speed;
    float rotation;
    Texture2D texture;
    int frameWidth, frameHeight, currentFrame;
    float animTimer;

    float scale; // YENİ DEĞİŞKEN
};