#include "Tower.h"
#include "raymath.h"
#include "Audio.h" // Ses sistemini dahil ettik

// Constructor
Tower::Tower(Vector2 pos, Texture2D tex, Texture2D projTex, TowerType type)
    : position(pos), texture(tex), projTexture(projTex), type(type),
    level(1), cooldown(0.0f), range(0.0f), damage(0), fireRate(0.0f), cost(0)
{
    // Kule Tipine Göre Özellikler
    if (type == TowerType::ARCHER) {
        range = 250.0f; damage = 20; fireRate = 1.0f; cost = 100;
    }
    else if (type == TowerType::MELEE) {
        range = 100.0f; damage = 15; fireRate = 1.5f; cost = 120;
    }
    else if (type == TowerType::ICE) {
        range = 180.0f; damage = 7.5; fireRate = 2.0f; cost = 150;
    }
}

void Tower::Update(float dt, std::vector<Enemy>& enemies, std::vector<Projectile>& projectiles) {
    cooldown -= dt;

    // Kule ateşe hazırsa
    if (cooldown <= 0.0f) {
        for (Enemy& e : enemies) {
            if (!e.IsAlive()) continue;

            // Düşman menzilde mi?
            if (CheckCollisionCircles(position, range, e.GetPosition(), e.GetRadius())) {

                ProjectileType pType = ProjectileType::ARROW;
                float projScale = 0.2f; // Mermi boyutu (Varsayılan)

                // 1. Kule Tipine Göre Mermi Ayarları
                if (type == TowerType::ICE) {
                    pType = ProjectileType::ICE;
                    projScale = 0.8f; // Buz mermisi biraz daha büyük
                }
                else if (type == TowerType::MELEE) {
                    pType = ProjectileType::MELEE;
                    projScale = 0.2f; // Melee "Vuruş Efekti" kocaman olsun
                }

                // 2. Saldırı Mantığı
                if (type == TowerType::MELEE) {
                    // --- MELEE: ANINDA HASAR VE EFEKT ---
                    e.TakeDamage(damage);

                    // Ses Çal (Hafif rastgelelik ile)
                    Audio::PlaySFX("sword_hit", 0.6f, 1.0f + (float)GetRandomValue(-2, 2) / 10.0f);

                    // Vuruş Efektini Düşmanın Üstünde Oluştur
                    projectiles.emplace_back(
                        e.GetPosition(), // Başlangıç: Düşmanın üstü
                        e.GetPosition(), // Hedef: Düşmanın üstü (Hareket etmeyecek)
                        0,               // Hasar: 0 (Burada zaten vurduk)
                        pType,
                        projTexture,
                        projScale        // Boyut
                    );
                }
                else {
                    // --- MENZİLLİ: MERMİ FIRLAT ---

                    // Ses Çal
                    if (type == TowerType::ARCHER) Audio::PlaySFX("arrow_shoot", 0.1f);
                    else if (type == TowerType::ICE) Audio::PlaySFX("ice_shoot", 0.1f);

                    // Mermiyi Oluştur
                    projectiles.emplace_back(
                        position,        // Başlangıç: Kule
                        e.GetPosition(), // Hedef: Düşman
                        damage,
                        pType,
                        projTexture,
                        projScale        // Boyut
                    );
                }

                cooldown = fireRate;
                break; // Tek seferde bir düşmana odaklan
            }
        }
    }
}

void Tower::Draw() const {
    // Kuleyi Çiz (Ayakları "position" noktasına gelecek şekilde)
    // 64x114 boyutunu kullanıyoruz, origin (32, 100) ayakları hizalar.
    DrawTexturePro(texture,
        { 0, 0, (float)texture.width, (float)texture.height },
        { position.x, position.y, 64, 114 },
        { 32, 100 },
        0.0f, WHITE);
}

void Tower::Upgrade() {
    level++;
    damage += 2;      // Hasar artır
    range += 10.0f;   // Menzil artır
    fireRate *= 0.9f; // Daha hızlı ateş et (%10 hızlan)

    // Upgrade sesi
    Audio::PlaySFX("build_tower", 1.2f);
}

bool Tower::IsClicked(Vector2 mousePos) const {
    // Tıklama kontrolü
    return CheckCollisionPointCircle(mousePos, position, 30.0f);
}