#include "Tower.h"
#include "raymath.h"

// Constructor: Mermi resmini alıp 'projTexture' değişkenine kaydediyoruz
Tower::Tower(Vector2 pos, Texture2D tex, Texture2D projTex, TowerType type)
    : position(pos), texture(tex), projTexture(projTex), type(type),
    level(1), cooldown(0.0f), range(0.0f), damage(0), fireRate(0.0f), cost(0)
{
    // Özellikler
    if (type == TowerType::ARCHER) { range = 250.0f; damage = 5; fireRate = 1.0f; cost = 100; }
    else if (type == TowerType::MELEE) { range = 100.0f; damage = 15; fireRate = 1.5f; cost = 75; }
    else if (type == TowerType::ICE) { range = 180.0f; damage = 2; fireRate = 2.0f; cost = 150; }
}

void Tower::Update(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles) {
    cooldown -= dt;
    if (cooldown <= 0.0f) {
        for (Enemy& e : enemies) {
            if (!e.IsAlive()) continue;

            if (CheckCollisionCircles(position, range, e.GetPosition(), e.GetRadius())) {

                ProjectileType pType = ProjectileType::ARROW;
                float projScale = 0.2f; 

                if (type == TowerType::ICE) {
                    pType = ProjectileType::ICE;
                    projScale = 1.2f; // Buz mermisi %20 daha büyük olsun
                }
                else if (type == TowerType::MELEE) {
                    pType = ProjectileType::MELEE;
                    projScale = 0.2f; // MELEE EFEKTİ 2 KAT BÜYÜK OLSUN!
                }

                if (type == TowerType::MELEE) {
                    e.TakeDamage(damage);
                    projectiles.emplace_back(
                        e.GetPosition(),
                        e.GetPosition(),
                        0,
                        pType,
                        projTexture,
                        projScale // <-- BURADA BOYUTU GÖNDERİYORUZ
                    );
                }
                else {
                    projectiles.emplace_back(
                        position,
                        e.GetPosition(),
                        damage,
                        pType,
                        projTexture,
                        projScale // <-- BURADA BOYUTU GÖNDERİYORUZ
                    );
                }

                cooldown = fireRate;
                break;
            }
        }
    }
}

void Tower::Draw() const {
    // Kuleyi çiz (Ayakları merkeze gelecek şekilde)
    DrawTexturePro(texture,
        { 0,0,(float)texture.width,(float)texture.height },
        { position.x, position.y, 64, 114 },
        { 32, 100 },
        0.0f, WHITE);
}

void Tower::Upgrade() {
    level++;
    damage += 2;
    range += 10.0f;
    fireRate *= 0.9f;
}

bool Tower::IsClicked(Vector2 mousePos) const {
    return CheckCollisionPointCircle(mousePos, position, 30.0f);
}