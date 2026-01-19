#include "Tower.h"
#include "raymath.h"
#include "Audio.h" 


Tower::Tower(Vector2 pos, Texture2D tex, Texture2D projTex, TowerType type)
    : position(pos), texture(tex), projTexture(projTex), type(type),
    level(1), cooldown(0.0f), range(0.0f), damage(0), fireRate(0.0f), cost(0)
{
    
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

    
    if (cooldown <= 0.0f) {
        for (Enemy& e : enemies) {
            if (!e.IsAlive()) continue;

            /* TARGET ACQUISITION :
             Checks if the enemy is within the tower's effective range using a circular collision check.
             This is an efficient check (Euclidean distance) suitable for high-frequency updates.*/
            if (CheckCollisionCircles(position, range, e.GetPosition(), e.GetRadius())) {

                ProjectileType pType = ProjectileType::ARROW;
                float projScale = 0.2f; 

                
                if (type == TowerType::ICE) {
                    pType = ProjectileType::ICE;
                    projScale = 0.8f; 
                }
                else if (type == TowerType::MELEE) {
                    pType = ProjectileType::MELEE;
                    projScale = 0.2f;
                }

                /* ATTACK LOGIC DISTINCTION :
                 MELEE towers act as "Hitscan" weapons; they apply damage immediately to the target 
                 and spawn a projectile only for visual effects.
                 RANGED towers (Archer/Ice) spawn a physical projectile entity that must travel 
                 to the target before damage is calculated.*/
                if (type == TowerType::MELEE) {
                   
                    e.TakeDamage(damage);

                    
                    Audio::PlaySFX("sword_hit", 0.6f, 1.0f + (float)GetRandomValue(-2, 2) / 10.0f);

                   
                    projectiles.emplace_back(
                        e.GetPosition(), 
                        e.GetPosition(), 
                        0,               
                        pType,
                        projTexture,
                        projScale        
                    );
                }
                else {
                    

                    
                    if (type == TowerType::ARCHER) Audio::PlaySFX("arrow_shoot", 0.1f);
                    else if (type == TowerType::ICE) Audio::PlaySFX("ice_shoot", 0.1f);

                    
                    projectiles.emplace_back(
                        position,        
                        e.GetPosition(), 
                        damage,
                        pType,
                        projTexture,
                        projScale        
                    );
                }
                /* RATE OF FIRE LIMITER :
                 Reset the cooldown timer and BREAK the loop immediately. 
                 This ensures the tower attacks only ONE enemy per frame, preventing it from 
                 damaging the entire wave simultaneously.*/
                cooldown = fireRate;
                break; 
            }
        }
    }
}

void Tower::Draw() const {
    /* RENDERING OFFSET :
     The 'origin' vector {32, 100} anchors the texture drawing to the bottom-center 
     of the sprite. This ensures the tower appears to stand "on" the tile 
     rather than floating above it in the isometric perspective.*/
    DrawTexturePro(texture,
        { 0, 0, (float)texture.width, (float)texture.height },
        { position.x, position.y, 64, 114 },
        { 32, 100 },
        0.0f, WHITE);
}

void Tower::Upgrade() {
    level++;
    damage += 2;      
    range += 10.0f;   
    fireRate *= 0.9f; 

   
    Audio::PlaySFX("build_tower", 1.2f);
}

bool Tower::IsClicked(Vector2 mousePos) const {
    
    return CheckCollisionPointCircle(mousePos, position, 30.0f);
}
