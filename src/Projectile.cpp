#include "Projectile.h"

Projectile::Projectile(Vector2 pos, Vector2 vel, int dmg)
    : position(pos), velocity(vel), damage(dmg), active(true)
{
}

void Projectile::Update(float dt) {
    if (!active) return;

   
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;

    
    if (position.x < -50 || position.x > 1300 ||
        position.y < -50 || position.y > 800) {
        active = false;
    }
}

void Projectile::Draw() const {
    if (active) {
        DrawCircleV(position, 4, YELLOW);
    }
}
