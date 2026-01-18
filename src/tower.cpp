#include "Tower.h"

Tower::Tower(Vector2 pos, Texture2D tex, TowerType type)
    : position(pos), texture(tex), type(type), timer(0.0f), level(1)
{
    if (type == TowerType::ARCHER) {
        range = 200.0f; damage = 10; cooldown = 1.0f; cost = 100;
    }
    else if (type == TowerType::MELEE) {
        range = 100.0f; damage = 20; cooldown = 1.5f; cost = 75;
    }
    else if (type == TowerType::ICE) {
        range = 150.0f; damage = 5; cooldown = 2.0f; cost = 150;
    }
}

void Tower::Update(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles) {
    timer += dt;
    if (timer >= cooldown) {
        int targetIndex = -1;

        for (int i = 0; i < enemies.size(); i++) {
            if (enemies[i].IsAlive()) {
                float dist = Vector2Distance(position, enemies[i].GetPosition());
                if (dist <= range) {
                    targetIndex = i;
                    break;
                }
            }
        }

        if (targetIndex != -1) {
            timer = 0.0f;
            ProjectileType pType = ProjectileType::ARROW;
            if (type == TowerType::ICE) pType = ProjectileType::ICE;
            else if (type == TowerType::MELEE) pType = ProjectileType::BOMB;

            // Projectile.h içindeki constructor'ı çağırır
            projectiles.emplace_back(position, enemies[targetIndex].GetPosition(), (int)damage, pType);
        }
    }
}

void Tower::Draw() const {
    Rectangle source = { 0, 0, (float)texture.width, (float)texture.height };
    Rectangle dest = { position.x, position.y, 64, 64 };
    Vector2 origin = { 32, 32 };
    DrawTexturePro(texture, source, dest, origin, 0.0f, WHITE);

    // Level göstergesi (Kule üzerine küçük sarı nokta)
    for (int i = 0; i < level; i++) {
        DrawCircle((int)position.x - 20 + (i * 10), (int)position.y + 25, 3, YELLOW);
    }
}

bool Tower::IsClicked(Vector2 mousePos) const {
    return CheckCollisionPointCircle(mousePos, position, 30.0f);
}

int Tower::GetUpgradeCost() const {
    return (int)(cost * 0.5f * level) + 50;
}

void Tower::Upgrade() {
    if (level >= 5) return;
    level++;
    damage *= 1.3f;
    range += 10.0f;
    if (cooldown > 0.5f) cooldown *= 0.9f;
}