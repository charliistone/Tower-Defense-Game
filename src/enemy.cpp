#include "Enemy.h"

Enemy::Enemy(EnemyType type, std::vector<Vector2>* path, Texture2D tex, float speedMult, int hpBonus)
    : position({ 0,0 }), path(path), currentPoint(0), texture(tex), type(type),
    alive(true), health(0), maxHealth(0), speed(0.0f), distanceTraveled(0.0f),
    manaReward(0), stunTimer(0.0f), slowTimer(0.0f), slowFactor(1.0f), frozen(false),
    currentFrame(0), animTimer(0.0f), facing(0)
{
    // Başlangıç pozisyonu
    if (path && !path->empty()) position = (*path)[0];

    // Temel Özellikler
    if (type == EnemyType::ORC) {
        maxHealth = 20; speed = 120.0f; manaReward = 5;
        damage = 10; // hasarı belirler
    }
    else if (type == EnemyType::URUK) {
        maxHealth = 50; speed = 90.0f; manaReward = 10;
        damage = 25; // hasarı belirler
    }
    else if (type == EnemyType::TROLL) {
        maxHealth = 300; speed = 50.0f; manaReward = 50;
        damage = 100; // hasarı belirler
    }

    // --- ZORLUK AYARLAMASI ---
    maxHealth += hpBonus;       // Level bonusunu ekle
    health = maxHealth;         // Canı fulle
    speed *= speedMult;         // Hız çarpanını uygula

    // Sprite Ayarları (3x4 Grid varsayımı)
    frameWidth = texture.width / 3;
    frameHeight = texture.height / 4;
}

void Enemy::Update(float dt) {
    if (!alive) return;

    // Hız ve Efekt Kontrolü
    float actualSpeed = speed;
    if (stunTimer > 0.0f) {
        stunTimer -= dt;
        actualSpeed = 0.0f;
        frozen = true;
    }
    else {
        frozen = false;
        if (slowTimer > 0.0f) {
            slowTimer -= dt;
            actualSpeed *= slowFactor;
        }
    }

    // Hareket Mantığı
    if (path && currentPoint < path->size()) {
        Vector2 target = (*path)[currentPoint];
        Vector2 dir = Vector2Subtract(target, position);
        float dist = Vector2Length(dir);

        // Yön Belirleme (Animasyon İçin)
        if (fabs(dir.x) > fabs(dir.y)) {
            facing = (dir.x > 0) ? 2 : 1; // 2: Sağ, 1: Sol
        }
        else {
            facing = (dir.y > 0) ? 0 : 3; // 0: Aşağı, 3: Yukarı
        }

        // İlerleme
        float moveStep = actualSpeed * dt;
        if (dist <= moveStep) {
            position = target;
            currentPoint++;
            distanceTraveled += dist;
        }
        else {
            Vector2 normDir = Vector2Normalize(dir);
            position = Vector2Add(position, Vector2Scale(normDir, moveStep));
            distanceTraveled += moveStep;
        }

        // Animasyon
        if (actualSpeed > 0) {
            animTimer += dt;
            if (animTimer >= 0.2f) {
                animTimer = 0.0f;
                currentFrame++;
                if (currentFrame >= 3) currentFrame = 0;
            }
        }
    }
}

void Enemy::Draw() const {
    if (!alive) return;

    float drawSize = 48.0f;
    if (type == EnemyType::TROLL) drawSize = 64.0f;

    Rectangle source;
    if (texture.width == texture.height) { // Tek kare resimse
        source = { 0, 0, (float)texture.width, (float)texture.height };
    }
    else { // Sprite sheet ise
        source = { (float)currentFrame * frameWidth, (float)facing * frameHeight, (float)frameWidth, (float)frameHeight };
    }

    Rectangle dest = { position.x, position.y, drawSize, drawSize };
    Vector2 origin = { drawSize / 2.0f, drawSize / 2.0f };

    Color tint = WHITE;
    if (stunTimer > 0.0f) tint = GOLD;
    else if (slowTimer > 0.0f) tint = SKYBLUE;

    DrawTexturePro(texture, source, dest, origin, 0.0f, tint);

    // Can Barı
    float pct = (float)health / (float)maxHealth;
    int barWidth = (int)drawSize;
    DrawRectangle((int)position.x - barWidth / 2, (int)position.y - (int)(drawSize / 2) - 10, barWidth, 6, RED);
    DrawRectangle((int)position.x - barWidth / 2, (int)position.y - (int)(drawSize / 2) - 10, (int)(barWidth * pct), 6, GREEN);
}

void Enemy::TakeDamage(int dmg) {
    health -= dmg;
    if (health <= 0) {
        health = 0;
        alive = false;
    }
}

void Enemy::ApplyStun(float duration) { stunTimer = duration; }
void Enemy::ApplySlow(float factor, float duration) { slowFactor = factor; slowTimer = duration; }