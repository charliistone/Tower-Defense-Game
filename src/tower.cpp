#include "Tower.h"

// DÜZELTME: Constructor 4 parametre alıyor ve projTex'i kaydediyor
Tower::Tower(Vector2 pos, Texture2D tex, Texture2D projTex, TowerType type)
    : position(pos), texture(tex), projTexture(projTex), type(type),
    level(1), cooldown(0.0f), range(0.0f), damage(0), fireRate(0.0f), cost(0)
{
    // Kule özelliklerini ayarla
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

                // Saldırı Türü Belirleme
                ProjectileType pType = ProjectileType::ARROW;
                if (type == TowerType::ICE) pType = ProjectileType::ICE;

                if (type == TowerType::MELEE) {
                    e.TakeDamage(damage); // Yakın dövüş anında vurur
                }
                else {
                    // DÜZELTME: Mermiyi oluştururken sınıftaki 'projTexture' değişkenini kullan
                    projectiles.emplace_back(position, e.GetPosition(), damage, pType, projTexture);
                }

                cooldown = fireRate; // Ateş hızına göre bekle
                break;
            }
        }
    }
}

void Tower::Draw() const {
    // Kule seviyesi göstergesi (Halka rengi)
    Color ringColor = GREEN;
    if (level == 2) ringColor = BLUE;
    if (level >= 3) ringColor = GOLD;

    DrawCircleLines((int)position.x, (int)position.y + 10, 30.0f, Fade(ringColor, 0.5f));

    // Kule Çizimi
    float drawWidth = 64.0f;
    float drawHeight = 114.0f;

    Rectangle source = { 0, 0, (float)texture.width, (float)texture.height };
    Rectangle dest = { position.x, position.y, drawWidth, drawHeight };
    Vector2 origin = { 32, 100 }; // Ayağını merkeze oturt

    DrawTexturePro(texture, source, dest, origin, 0.0f, WHITE);

    // Seviye Metni
    DrawText(TextFormat("Lv%d", level), (int)position.x - 10, (int)position.y - 40, 20, ringColor);
}

void Tower::Upgrade() {
    level++;
    damage += 5;
    range += 20.0f;
    fireRate *= 0.9f; // Daha hızlı ateş et (Süre azalır)
}

int Tower::GetUpgradeCost() const {
    return cost + (level * 50);
}

bool Tower::IsClicked(Vector2 mousePos) const {
    return CheckCollisionPointCircle(mousePos, position, 30.0f);
}