#include "Tower.h"

Tower::Tower(Vector2 pos, Texture2D tex)
    : position(pos), texture(tex),
      range(150), fireRate(1.0f),
      cooldown(0), damage(25) {}

void Tower::Update(float dt,
    std::vector<Enemy>& enemies,
    std::vector<Projectile>& projectiles)
{
    cooldown -= dt;
    if (cooldown > 0) return;

    for (Enemy& e : enemies) {
        if (!e.IsAlive()) continue;
        if (Vector2Distance(position, e.GetPosition()) <= range) {
            Vector2 dir = Vector2Normalize(
                Vector2Subtract(e.GetPosition(), position)
            );
            projectiles.emplace_back(position,
                Vector2Scale(dir, 300), damage);
            cooldown = 1.0f / fireRate;
            break;
        }
    }
}

void Tower::Draw() const {
    DrawTextureV(texture, {position.x-16, position.y-16}, WHITE);
    DrawCircleLines(position.x, position.y, range, Fade(BLUE,0.2f));
}

void Tower::Draw() const {
    DrawTextureV(texture, {position.x - 16, position.y - 16}, WHITE);
    DrawCircleLines(position.x, position.y, range, Fade(BLUE, 0.25f));
}
