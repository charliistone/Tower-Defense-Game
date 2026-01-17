#include "Tower.h"

// --- FIX: INITIALIZE EVERY VARIABLE HERE ---
Tower::Tower(Vector2 pos, Texture2D tex, TowerType type)
    : position(pos), texture(tex), type(type),
    level(1), cooldown(0.0f), range(0.0f), damage(0), fireRate(0.0f), cost(0)
{
    if (type == TowerType::ARCHER) {
        range = 200.0f; damage = 5; fireRate = 1.0f; cost = 100;
    }
    else if (type == TowerType::MELEE) {
        range = 100.0f; damage = 15; fireRate = 1.5f; cost = 75;
    }
    else if (type == TowerType::ICE) {
        range = 150.0f; damage = 2; fireRate = 2.0f; cost = 150;
    }
}

void Tower::Update(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles) {
    cooldown -= dt;
    if (cooldown <= 0.0f) {
        for (Enemy& e : enemies) {
            if (!e.IsAlive()) continue;

            if (CheckCollisionCircles(position, range, e.GetPosition(), e.GetRadius())) {

                // Determine Projectile Type
                ProjectileType pType = ProjectileType::ARROW;
                if (type == TowerType::ICE) pType = ProjectileType::ICE;

                if (type == TowerType::MELEE) {
                    e.TakeDamage(damage); // Instant hit
                }
                else {
                    projectiles.emplace_back(position, e.GetPosition(), damage, pType);
                }

                cooldown = fireRate;
                break;
            }
        }
    }
}

void Tower::Draw() const {
    DrawCircleLines((int)position.x, (int)position.y, 35.0f, GRAY);

    Rectangle source = { 0, 0, (float)texture.width, (float)texture.height };
    Rectangle dest = { position.x, position.y - 16, 64, 64 };
    Vector2 origin = { 32, 32 };

    Color tint = WHITE;
    if (type == TowerType::ICE) tint = SKYBLUE;
    if (type == TowerType::MELEE) tint = RED;

    DrawTexturePro(texture, source, dest, origin, 0.0f, tint);
}

void Tower::Upgrade() {
    level++;
    damage += 5;
    fireRate *= 0.85f;
    range += 20.0f;
}

int Tower::GetUpgradeCost() const {
    return cost * level;
}

bool Tower::IsClicked(Vector2 mousePos) const {
    return CheckCollisionPointCircle(mousePos, position, 32.0f);
}