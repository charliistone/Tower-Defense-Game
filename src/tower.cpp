#include "Tower.h"
#include "raymath.h"

Tower::Tower(Vector2 pos, Texture2D tex, TowerType t)
    : position(pos), texture(tex), type(t), cooldown(0.0f), level(1)
{
    // Base Stats
    switch (type) {
    case TowerType::ARCHER:
        range = 200.0f; damage = 25; fireRate = 1.0f; cost = 100;
        break;
    case TowerType::ICE:
        range = 150.0f; damage = 10; fireRate = 1.5f; cost = 150;
        break;
    case TowerType::MELEE:
        range = 100.0f; damage = 50; fireRate = 0.8f; cost = 75;
        break;
    }
}

// --- NEW: UPGRADE LOGIC ---
void Tower::Upgrade() {
    level++;

    // Improve stats based on Type
    if (type == TowerType::ARCHER) {
        damage += 15;   // Big damage boost
        range += 20.0f; // Can see further
    }
    else if (type == TowerType::ICE) {
        fireRate *= 0.85f; // Shoots faster (more slowing!)
        range += 10.0f;
    }
    else if (type == TowerType::MELEE) {
        damage += 30;   // Huge damage boost
        fireRate *= 0.9f; // Slightly faster
    }
}

int Tower::GetUpgradeCost() const {
    // Upgrade Price = Base Cost * Current Level
    // Level 1->2 costs 100g. Level 2->3 costs 200g.
    return cost * level;
}

bool Tower::IsClicked(Vector2 mousePos) const {
    // Hitbox check (approx 40 pixels wide)
    return CheckCollisionPointCircle(mousePos, position, 40.0f);
}
// --------------------------

void Tower::Update(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles) {
    cooldown -= dt;
    if (cooldown > 0) return;

    const Enemy* target = nullptr;
    for (const auto& e : enemies) {
        if (!e.IsAlive()) continue;
        if (Vector2Distance(position, e.GetPosition()) <= range) {
            target = &e;
            break;
        }
    }

    if (target) {
        Vector2 dir = Vector2Subtract(target->GetPosition(), position);
        dir = Vector2Normalize(dir);

        ProjectileType pType = (type == TowerType::ICE) ? ProjectileType::ICE : ProjectileType::NORMAL;
        float speed = (type == TowerType::MELEE) ? 1200.0f : 900.0f;

        projectiles.emplace_back(position, Vector2Scale(dir, speed), damage, pType);
        cooldown = fireRate;
    }
}

void Tower::Draw() const {
    Color tint = WHITE;
    if (type == TowerType::ICE) tint = SKYBLUE;
    if (type == TowerType::MELEE) tint = RED;

    // Draw the Tower
    float scale = 3.0f;
    Rectangle source = { 0, 0, (float)texture.width, (float)texture.height };
    Rectangle dest = { position.x, position.y, texture.width * scale, texture.height * scale };
    DrawTexturePro(texture, source, dest, { dest.width / 2, dest.height / 2 }, 0.0f, tint);

    // --- NEW: Draw Level Indicator ---
    // Draw tiny stars or text above the tower to show Level
    DrawText(TextFormat("Lvl %d", level), position.x - 20, position.y - 50, 20, YELLOW);
}