#include "Tower.h"

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

                ProjectileType pType = ProjectileType::ARROW;
                if (type == TowerType::ICE) pType = ProjectileType::ICE;

                if (type == TowerType::MELEE) {
                    e.TakeDamage(damage); 
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
    
    Color ringColor = GRAY;
    if (level == 2) ringColor = BLUE;
    if (level >= 3) ringColor = GOLD;

   
    DrawCircleLines((int)position.x, (int)position.y + 10, 30.0f, Fade(ringColor, 0.5f));

  
    Color tint = WHITE;

 
    float drawWidth = 64.0f;
    float drawHeight = 114.0f;

    Rectangle source = { 0, 0, (float)texture.width, (float)texture.height };
    Rectangle dest = { position.x, position.y, drawWidth, drawHeight };

 
    Vector2 origin = { drawWidth / 2.0f, drawHeight - 20.0f };

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
