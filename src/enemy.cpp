#include "Enemy.h"

Enemy::Enemy(EnemyType type, std::vector<Vector2>* path, Texture2D tex, float speedMult, int hpBonus)
    : position({ 0,0 }), path(path), currentPoint(0), texture(tex), type(type),
    alive(true), health(0), maxHealth(0), speed(0.0f), distanceTraveled(0.0f),
    manaReward(0), stunTimer(0.0f), slowTimer(0.0f), slowFactor(1.0f), frozen(false),
    currentFrame(0), animTimer(0.0f), facing(0)
{
    
    if (path && !path->empty()) position = (*path)[0];

    /* STATS INITIALIZATION :
     Defines the base attributes (Health, Speed, Reward, Damage to Castle) for each enemy type.
     "speed" is in pixels per second. "manaReward" is the amount of Uruk Blood gained on kill.
     "damage" is how much castle health is lost if this enemy reaches the end.*/
    if (type == EnemyType::ORC) {
        maxHealth = 20; speed = 120.0f; manaReward = 5;
        damage = 10; 
    }
    else if (type == EnemyType::URUK) {
        maxHealth = 50; speed = 90.0f; manaReward = 10;
        damage = 25; 
    }
    else if (type == EnemyType::TROLL) {
        maxHealth = 300; speed = 50.0f; manaReward = 50;
        damage = 100; 
    }
    else if (type == EnemyType::GROND) {
        maxHealth = 1000; speed = 25.0f;manaReward = 500; 
        damage = 500; 
    }
    else if (type == EnemyType::NAZGUL) {
        maxHealth = 2500; 
        speed = 50.0f;    
        manaReward = 1000;
        damage = 9999;    
    }
    else if (type == EnemyType::COMMANDER) {
        maxHealth = 600;  
        speed = 60.0f;    
        manaReward = 150; 
        damage = 50;      
    }

    
    maxHealth += hpBonus;       
    health = maxHealth;         
    speed *= speedMult;        

   
    frameWidth = texture.width / 3;
    frameHeight = texture.height / 4;
}

void Enemy::Update(float dt) {
    if (!alive) return;

    /* STATUS EFFECT LOGIC :
     Prioritizes Stun over Slow.
     If Stunned: Speed is set to 0.
     If Slowed (and not stunned): Speed is multiplied by a factor (e.g., 0.5 for 50% slow).
     The timers are decremented by 'dt' (delta time) every frame.*/
    float actualSpeed = speed;
    if (stunTimer > 0.0f) {
        stunTimer -= dt;
        actualSpeed = 0.0f;
        frozen = true;
    }
    else {
        frozen = false;
        if (slowTimer > 0.0f) {
            slowTimer -= dt;
            actualSpeed *= slowFactor;
        }
    }

     /* PATH FOLLOWING ALGORITHM :
     Calculates the direction vector towards the current target node in the path.
     If the enemy is close enough to the target (dist <= moveStep), it snaps to the target
     and increments the 'currentPoint' index to target the next node in the path vector.
     Otherwise, it moves along the normalized direction vector.*/
    if (path && currentPoint < path->size()) {
        Vector2 target = (*path)[currentPoint];
        Vector2 dir = Vector2Subtract(target, position);
        float dist = Vector2Length(dir);

        /* ANIMATION STATE :
         Determines the 'facing' direction (0=Down, 1=Left, 2=Right, 3=Up) based on the movement vector.
         This index corresponds to the row in the sprite sheet.*/
        if (fabs(dir.x) > fabs(dir.y)) {
            facing = (dir.x > 0) ? 2 : 1; 
        }
        else {
            facing = (dir.y > 0) ? 0 : 3; 
        }

        
        float moveStep = actualSpeed * dt;
        if (dist <= moveStep) {
            position = target;
            currentPoint++;
            distanceTraveled += dist;
        }
        else {
            Vector2 normDir = Vector2Normalize(dir);
            position = Vector2Add(position, Vector2Scale(normDir, moveStep));
            distanceTraveled += moveStep;
        }

        
        if (actualSpeed > 0) {
            animTimer += dt;
            if (animTimer >= 0.2f) {
                animTimer = 0.0f;
                currentFrame++;
                if (currentFrame >= 3) currentFrame = 0;
            }
        }
    }
}

void Enemy::Draw() const {
    if (!alive) return;

    float drawSize = 48.0f;
    if (type == EnemyType::TROLL) drawSize = 64.0f;
    else if (type == EnemyType::GROND) drawSize = 100.0f;
    else if (type == EnemyType::NAZGUL) drawSize = 130.0f; 
    else if (type == EnemyType::COMMANDER) {
        drawSize = 60.0f; 
    }

    Rectangle source;
    if (texture.width == texture.height) { // Tek kare resimse
        source = { 0, 0, (float)texture.width, (float)texture.height };
    }
    else { 
        source = { (float)currentFrame * frameWidth, (float)facing * frameHeight, (float)frameWidth, (float)frameHeight };
    }

    Rectangle dest = { position.x, position.y, drawSize, drawSize };
    Vector2 origin = { drawSize / 2.0f, drawSize / 2.0f };

    Color tint = WHITE;
    if (stunTimer > 0.0f) tint = GOLD;
    else if (slowTimer > 0.0f) tint = SKYBLUE;

    DrawTexturePro(texture, source, dest, origin, 0.0f, tint);

   

    int offset = 15;
    if (type == EnemyType::GROND) offset = 40;
    else if (type == EnemyType::NAZGUL) offset = 30;
    
    /* RENDER LOGIC :
     Draws the enemy sprite centered on its logical position.
     Also renders a dynamic health bar above the sprite, scaling its green width 
     based on the current health percentage.*/
    float pct = (float)health / (float)maxHealth;
    int barWidth = (int)drawSize;
    DrawRectangle((int)position.x - barWidth / 2, (int)position.y - (int)(drawSize / 2) - 10, barWidth, 6, RED);
    DrawRectangle((int)position.x - barWidth / 2, (int)position.y - (int)(drawSize / 2) - 10, (int)(barWidth * pct), 6, GREEN);
}

void Enemy::TakeDamage(int dmg) {
    health -= dmg;
    if (health <= 0) {
        health = 0;
        alive = false;
    }
}

void Enemy::ApplyStun(float duration) { stunTimer = duration; }
void Enemy::ApplySlow(float factor, float duration) { slowFactor = factor; slowTimer = duration; }
