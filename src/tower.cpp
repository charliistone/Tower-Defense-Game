#include "Tower.h"

Tower::Tower(Vector2 pos, Texture2D tex)
    : position(pos), texture(tex),
    range(150), fireRate(1.0f),
    cooldown(0), damage(25) {
}

void Tower::Update(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles)
{
    cooldown -= dt;
    if (cooldown > 0) return;

    for (Enemy& e : enemies) {
        if (!e.IsAlive()) continue;

        // Check distance
        if (Vector2Distance(position, e.GetPosition()) <= range) {

            // Calculate direction vector
            Vector2 dir = Vector2Subtract(e.GetPosition(), position);
            dir = Vector2Normalize(dir);

            // Shoot! Add a new projectile to the list
            // Scale(dir, 300) means the bullet moves at 300 pixels/second
            projectiles.emplace_back(position, Vector2Scale(dir, 900), damage);

            // Reset cooldown
            cooldown = 1.0f / fireRate;
            break; // Only shoot one enemy per update
        }
    }
}

void Tower::Draw() const {
    // --- FIX START: Sprite Slicing ---
    // Again, assuming a 4x4 grid. Change to 3.0f or 1.0f if your image is different.
    float columns = 4.0f;
    float rows = 4.0f;

    float frameWidth = texture.width / columns;
    float frameHeight = texture.height / rows;

    // Cut out the top-left sprite
    Rectangle sourceRec = { 0.0f, 0.0f, frameWidth, frameHeight };

    // Draw centered on position
    Rectangle destRec = { position.x, position.y, frameWidth, frameHeight };
    Vector2 origin = { frameWidth / 2.0f, frameHeight / 2.0f };

    DrawTexturePro(texture, sourceRec, destRec, origin, 0.0f, WHITE);
    // --- FIX END ---

    // Draw Range Circle (Debugging)
    DrawCircleLines(position.x, position.y, range, Fade(BLUE, 0.25f));
}