#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_boss_01.h"
#include "z64rumble.h"
#include "z64shrink_window.h"
#include "attributes.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "overlays/actors/ovl_Door_Warp1/z_door_warp1.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "overlays/actors/ovl_En_Tanron1/z_en_tanron1.h"
#include "overlays/actors/ovl_Item_B_Heart/z_item_b_heart.h"

#define TIMER_CURRENT_ACTION 0
#define ODOLWA_MAX_HEALTH 20

static s32 healCounter = 0;
static s32 dodgeTimer = 0;

static Color_RGBA8 sDustPrimColor = { 60, 50, 20, 255 };

static Color_RGBA8 sDustEnvColor = { 40, 30, 30, 255 };

void Boss01_IntroCutscene(Boss01* this, PlayState* play);
void Boss01_SummonBugsCutscene(Boss01* this, PlayState* play);
void Boss01_SetupWait(Boss01* this, PlayState* play, u8 waitType);
void Boss01_SetupSpinAttack(Boss01* this, PlayState* play);
void Boss01_SetupDanceBeforeAttack(Boss01* this, PlayState* play);
void Boss01_Run(Boss01* this, PlayState* play);
void Boss01_Jump(Boss01* this, PlayState* play);
void Boss01_JumpLand(Boss01* this, PlayState* play);
void Boss01_SetupVerticalSlash(Boss01* this, PlayState* play);
void Boss01_SetupHorizontalSlash(Boss01* this, PlayState* play);
void Boss01_SetupKick(Boss01* this, PlayState* play);
void Boss01_SetupShieldBash(Boss01* this, PlayState* play);
void Boss01_Damaged(Boss01* this, PlayState* play);

static Vec3f sFallingBlockSfxPos = { 0.0f, 1000.0f, 0.0f };

typedef enum {
    // Named based on the fact that everything with this damage effect deals zero damage. If this effect is given to an
    // attack that deals non-zero damage, it will behave exactly like ODOLWA_DMGEFF_DAMAGE.
    /* 0x0 */ ODOLWA_DMGEFF_IMMUNE,

    // Deals no damage, but turns Odolwa blue, stops all animations, and makes him wait in place for 40 frames.
    /* 0x1 */ ODOLWA_DMGEFF_STUN,

    // Deals damage and surrounds Odolwa with fire.
    /* 0x2 */ ODOLWA_DMGEFF_FIRE,

    // Behaves exactly like ODOLWA_DMGEFF_STUN, but also surrounds Odolwa with ice.
    /* 0x3 */ ODOLWA_DMGEFF_FREEZE,

    // Deals damage and surrounds Odolwa with yellow light orbs.
    /* 0x4 */ ODOLWA_DMGEFF_LIGHT_ORB,

    // Behaves exactly like ODOLWA_DMGEFF_STUN, but also surrounds Odolwa in electric sparks.
    /* 0xB */ ODOLWA_DMGEFF_ELECTRIC_STUN = 0xB,

    // Deals damage and surrounds Odolwa with blue light orbs.
    /* 0xC */ ODOLWA_DMGEFF_BLUE_LIGHT_ORB,

    // Deals damage and has no special effect.
    /* 0xD */ ODOLWA_DMGEFF_DAMAGE,

    // Deals damage and checks the timer that tracks how long Odolwa should be in his damaged state. If the timer is 5
    // or more, it will reset the timer to 20 frames keep Odolwa in the damaged state for longer. If the timer is 4 or
    // less, it will disable Odolwa's collision for 20 frames to ensure he can jump away without taking further damage.
    /* 0xE */ ODOLWA_DMGEFF_DAMAGE_TIMER_CHECK,

    // Deals no damage, but makes Odolwa play his dazed animation for 70 frames and be vulnerable to attacks.
    /* 0xF */ ODOLWA_DMGEFF_DAZE
} OdolwaDamageEffect;

typedef enum {
    /*   0 */ ODOLWA_WAIT_READY,
    /*   1 */ ODOLWA_WAIT_SPIN_SWORD,
    /*   2 */ ODOLWA_WAIT_VERTICAL_HOP,
    /*   3 */ ODOLWA_WAIT_SHAKE_DANCE,
    /*   4 */ ODOLWA_WAIT_UP_AND_DOWN_DANCE,
    /*   5 */ ODOLWA_WAIT_ARM_SWING_DANCE,
    /*   6 */ ODOLWA_WAIT_THRUST_ATTACK,
    /*   7 */ ODOLWA_WAIT_DOUBLE_SLASH,
    /*   8 */ ODOLWA_WAIT_SIDE_TO_SIDE_HOP,
    /*   9 */ ODOLWA_WAIT_SIDE_TO_SIDE_DANCE,
    /*  10 */ ODOLWA_WAIT_SPIN_DANCE,
    /*  11 */ ODOLWA_WAIT_JUMP_DANCE,
    /*  12 */ ODOLWA_WAIT_MAX,

    // This doesn't correspond to an actual wait action that Odolwa can perform, but it can be passed as a parameter to
    // Boss01_SetupWait to randomly select between one of Odolwa's available wait types (as well as having a random
    // chance to summon moths instead), assuming that the fight is in its second phase.
    /* 100 */ ODOLWA_WAIT_RANDOM = 100
} OdolwaWaitType;

typedef enum {
    // There are no AT colliders enabled; the player can pass through the sword without reacting or taking any damage.
    /* 0 */ ODOLWA_SWORD_STATE_INACTIVE,

    // The sword's two AT colliders are enabled; the player will be knocked back and take damage if they touch the
    // sword. There is a third collider originating from Odolwa's pelvis that is also active, but it is offset very far
    // out-of-bounds in this state, so the player can never touch it.
    /* 1 */ ODOLWA_SWORD_STATE_ACTIVE,

    // Similar to the previous state, but the pelvis collider is now placed on the floor in front of Odolwa.
    /* 2 */ ODOLWA_SWORD_STATE_HORIZONTAL_SLASH
} OdolwaSwordState;

extern f32 sOdolwaSwordTrailPosX;
extern f32 sOdolwaSwordTrailPosY;
extern f32 sOdolwaSwordTrailPosZ;
extern f32 sOdolwaSwordTrailRotX;
extern f32 sOdolwaSwordTrailRotY;
extern f32 sOdolwaSwordTrailRotZ;
extern f32 sOdolwaSwordTrailAlpha;

static f32 sSwordTrailAngularRangeDivisor = 10.0f;

extern void Boss01_SpawnDustAtFeet(Boss01* this, PlayState* play, u8 dustSpawnFrameMask);

RECOMP_HOOK("Boss01_DanceBeforeAttack") void DanceLess(Boss01* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->timers[TIMER_CURRENT_ACTION]--;
        break;

    case 1:
        this->timers[TIMER_CURRENT_ACTION] = 0;
        break;

    default:
        break;
    }
}


RECOMP_PATCH void Boss01_SelectAttack(Boss01* this, PlayState* play, u8 mustAttack) {
    Player* player = GET_PLAYER(play);

    int Difficulty = (int)recomp_get_config_double("diff_option");

    float danceChance = 0.2f;
    float closeRangeDistance = 150.0f;
    float spinChance = 0.75f;

    switch (Difficulty) {
    case 0:
        danceChance = 0.15f;
        closeRangeDistance = 150.0f;
        spinChance = 0.825f;
        break;

    case 1:
        danceChance = 0.10f;
        closeRangeDistance = 150.0f;
        spinChance = 0.90f;
        break;

    default:
        break;
    }

    if (player->actor.world.pos.y > 200.0f) {
        Boss01_SetupWait(this, play, ODOLWA_WAIT_RANDOM);
    }
    else if ((Rand_ZeroOne() < danceChance) && !mustAttack) {
        // When Odolwa is done dancing, this function calls Boss01_SelectAttack with mustAttack set to true, so he will
        // be guaranteed to choose an attack later, so long as the player isn't too far off the ground.
        Boss01_SetupDanceBeforeAttack(this, play);
    }
    else if (this->actor.xzDistToPlayer <= 250.0f) {
        if (this->actor.xzDistToPlayer <= closeRangeDistance) {
            if (Rand_ZeroOne() < 0.5f) {
                Boss01_SetupKick(this, play);
            }
            else {
                Boss01_SetupShieldBash(this, play);
            }
        }
        else {
            Boss01_SetupHorizontalSlash(this, play);
        }
    }
    else if (((s8)this->actor.colChkInfo.health < (ODOLWA_MAX_HEALTH /2)) && (Rand_ZeroOne() < spinChance)) {
        Boss01_SetupSpinAttack(this, play);
    }
    else {
        Boss01_SetupVerticalSlash(this, play);
    }
}


RECOMP_PATCH void Boss01_VerticalSlash(Boss01* this, PlayState* play) {
    Vec3f additionalVelocity;
    Player* player = GET_PLAYER(play);

    SkelAnime_Update(&this->skelAnime);

    int Difficulty = (int)recomp_get_config_double("diff_option");

    f32 predictFactor = (Difficulty == 1) ? 1.0f : 0.66f;
    f32 distanceFactor = (Difficulty == 1) ? 3.0f : 2.0f;
    f32 dashSpeed = 25.0f * distanceFactor;

    if (this->skelAnime.curFrame < 10.0f) {
        Vec3f predictedPos;
        f32 dx = player->actor.world.pos.x - this->actor.world.pos.x;
        f32 dz = player->actor.world.pos.z - this->actor.world.pos.z;
        f32 distance = sqrtf(SQ(dx) + SQ(dz));
        f32 travelTime = distance / dashSpeed;

        travelTime = CLAMP(travelTime, 0.0f, 15.0f);

        predictedPos.x = player->actor.world.pos.x +
            (player->actor.velocity.x * travelTime * predictFactor);
        predictedPos.y = player->actor.world.pos.y;
        predictedPos.z = player->actor.world.pos.z +
            (player->actor.velocity.z * travelTime * predictFactor);

        s16 targetYaw = Math_Vec3f_Yaw(&this->actor.world.pos, &predictedPos);

        Math_SmoothStepToS(&this->actor.world.rot.y, targetYaw, 5, 0x4000, 0x100);
        this->actor.shape.rot.y = this->actor.world.rot.y;
    }

    if ((this->skelAnime.curFrame >= 10.0f) && (this->skelAnime.curFrame <= 15.0f)) {
        this->isPerformingVerticalSlash = true;
    }

    if ((this->timers[TIMER_CURRENT_ACTION] >= 7) && (this->timers[TIMER_CURRENT_ACTION] < 13)) {
        Vec3f predictedPos;
        f32 dx = player->actor.world.pos.x - this->actor.world.pos.x;
        f32 dz = player->actor.world.pos.z - this->actor.world.pos.z;
        f32 distance = sqrtf(SQ(dx) + SQ(dz));
        f32 travelTime = distance / dashSpeed;

        travelTime = CLAMP(travelTime, 0.0f, 15.0f);

        predictedPos.x = player->actor.world.pos.x +
            (player->actor.velocity.x * travelTime * predictFactor);
        predictedPos.y = player->actor.world.pos.y;
        predictedPos.z = player->actor.world.pos.z +
            (player->actor.velocity.z * travelTime * predictFactor);

        s16 targetYaw = Math_Vec3f_Yaw(&this->actor.world.pos, &predictedPos);

        Math_SmoothStepToS(&this->actor.world.rot.y, targetYaw, 5, 0x4000, 0x100);
        this->actor.shape.rot.y = this->actor.world.rot.y;

        Matrix_RotateYF(BINANG_TO_RAD_ALT(this->actor.world.rot.y), MTXMODE_NEW);
        Matrix_MultVecZ(dashSpeed, &additionalVelocity);
        this->additionalVelocityX = additionalVelocity.x;
        this->additionalVelocityZ = additionalVelocity.z;
        Boss01_SpawnDustAtFeet(this, play, 0);
    }

    sOdolwaSwordTrailPosX = 0.0f;
    sOdolwaSwordTrailPosY = 90.0f;
    sOdolwaSwordTrailPosZ = -70.0f;
    sOdolwaSwordTrailRotX = 0.4712388f;
    sOdolwaSwordTrailRotY = M_PIf;
    sOdolwaSwordTrailRotZ = 1.7278761f;

    if (Animation_OnFrame(&this->skelAnime, 12.0f)) {
        sOdolwaSwordTrailAlpha = 255.0f;
        sSwordTrailAngularRangeDivisor = 100.0f;
    }

    if (Animation_OnFrame(&this->skelAnime, 13.0f)) {
        sSwordTrailAngularRangeDivisor = 20.0f;
    }

    if (Animation_OnFrame(&this->skelAnime, 14.0f)) {
        sSwordTrailAngularRangeDivisor = 7.0f;
    }

    if (Animation_OnFrame(&this->skelAnime, 7.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_MIBOSS_DASH_OLD);
    }

    if (Animation_OnFrame(&this->skelAnime, 10.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_MIBOSS_SWORD_OLD);
    }

    if (Animation_OnFrame(&this->skelAnime, this->animEndFrame)) {
        Boss01_SetupWait(this, play, ODOLWA_WAIT_RANDOM);
        this->additionalVelocityZ = 0.0f;
        this->additionalVelocityX = 0.0f;
    }

    this->swordState = ODOLWA_SWORD_STATE_ACTIVE;
    this->swordAndShieldCollisionEnabled = true;
}


RECOMP_PATCH void Boss01_JumpSquat(Boss01* this, PlayState* play) {
    s32 pad[2];
    u8 i;
    Vec3f additionalVelocity;
    Player* player = GET_PLAYER(play);

    SkelAnime_Update(&this->skelAnime);

    int Difficulty = (f32)recomp_get_config_double("diff_option");

    if (this->timers[TIMER_CURRENT_ACTION] == 0) {
        f32 flightTime;
        f32 dx;
        f32 dz;
        f32 distance;
        f32 target = this->shouldPerformFallingSlash ? 12.0f : 10.0f;

        this->actionFunc = Boss01_Jump;
        Animation_MorphToPlayOnce(&this->skelAnime, &gOdolwaJumpAnim, this->animMorphFrames1);
        this->actor.velocity.y = 35.0f;
        this->actor.gravity = -2.5f;
        Matrix_RotateYS(this->actor.world.rot.y, MTXMODE_NEW);

        dx = player->actor.world.pos.x - this->actor.world.pos.x;
        dz = player->actor.world.pos.z - this->actor.world.pos.z;
        distance = sqrtf((dx * dx) + (dz * dz));

        if (distance > 150.0f) {
            flightTime = (this->actor.velocity.y + sqrtf(
                (this->actor.velocity.y * this->actor.velocity.y) -
                (2.0f * this->actor.gravity * (40.0f - this->actor.world.pos.y)))) /
                -this->actor.gravity;

            dx = player->actor.world.pos.x + player->actor.velocity.x * flightTime - this->actor.world.pos.x;
            dz = player->actor.world.pos.z + player->actor.velocity.z * flightTime - this->actor.world.pos.z;

            dx *= (Difficulty == 1.0f) ? 1.0f : 0.6f;
            dz *= (Difficulty == 1.0f) ? 1.0f : 0.6f;

            this->additionalVelocityX = dx / flightTime;
            this->additionalVelocityZ = dz / flightTime;
        }
        else {
            Matrix_MultVecZ(target, &additionalVelocity);
            this->additionalVelocityX = additionalVelocity.x;
            this->additionalVelocityZ = additionalVelocity.z;
        }

        for (i = 0; i < 5; i++) {
            Boss01_SpawnDustAtFeet(this, play, 0);
        }

        Actor_PlaySfx(&this->actor, NA_SE_EN_MIBOSS_JUMP1);
        Actor_PlaySfx(&this->actor, NA_SE_EN_MIBOSS_JUMP2 - SFX_FLAG);
        this->disableCollisionTimer = 5;
    }

    this->swordAndShieldCollisionEnabled = true;
}


RECOMP_HOOK("Boss01_Run") void RunningInThe90s(Boss01* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    int Difficulty = (int)recomp_get_config_double("diff_option");

    f32 diffX;
    f32 diffZ;
    f32 targetDist;
    f32 targetX;
    f32 targetZ;
    s16 targetYaw;

    f32 offset = 75.0f;
    f32 sinY = Math_SinS(player->actor.world.rot.y);
    f32 cosY = Math_CosS(player->actor.world.rot.y);

    targetX = player->actor.world.pos.x + (cosY * offset);
    targetZ = player->actor.world.pos.z - (sinY * offset);

    diffX = targetX - this->actor.world.pos.x;
    diffZ = targetZ - this->actor.world.pos.z;
    targetDist = sqrtf((diffX * diffX) + (diffZ * diffZ));

    if (Actor_WorldDistXZToPoint(&this->actor, &player->actor.world.pos) <
        ((Difficulty == 1) ? 350.0f : 300.0f) &&
        targetDist > 80.0f) {
        targetYaw = Math_Atan2S_XY(diffZ, diffX);

        Math_ApproachS(&this->actor.world.rot.y, targetYaw, 0xA, 0x1000);
    }
}


RECOMP_HOOK("Boss01_Update") void OdolwaUpdate(Actor* thisx, PlayState* play2) {

    Boss01* this = (Boss01*)thisx;


    int Difficulty = (int)recomp_get_config_double("diff_option");

    float speedMultiplier = 1.0f;
    s32 dtMax;

    switch (Difficulty) {
    case 0:
        speedMultiplier = 1.33f;
        dtMax = 120;
        break;

    case 1:
        if (this->actor.colChkInfo.damage != 1) {
            this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        }
        speedMultiplier = 2.0f;
        dtMax = 60;
        break;

    default:
        break;
    }

    if (this->actor.colChkInfo.health != 0 && this->actionFunc != Boss01_Jump && this->actionFunc != Boss01_IntroCutscene && this->actionFunc != Boss01_SummonBugsCutscene && this->actionFunc != Boss01_Run) {
        this->skelAnime.playSpeed = speedMultiplier;
    }

    if (this->actor.colChkInfo.health > 0) {
        healCounter++;
        dodgeTimer++;

        if (healCounter >= 360) {
            this->actor.colChkInfo.health += 1;

            if (this->actor.colChkInfo.health > ODOLWA_MAX_HEALTH) {
                this->actor.colChkInfo.health = ODOLWA_MAX_HEALTH;
            }

            healCounter = 0;
        }

        if (dodgeTimer >= dtMax) {
            this->canGuardOrEvade = true;
            dodgeTimer = 0;
        }
    }
}



RECOMP_HOOK("Boss01_Damaged") void GoAwayLink(Boss01* this, PlayState* play) {

    this->timers[TIMER_CURRENT_ACTION]--;

    healCounter = healCounter - 40;
    if (healCounter <= 0) healCounter = 0;
}


RECOMP_HOOK("Boss01_Bug_Update") void DamageReductionBug(Actor* thisx, PlayState* play) {

    Boss01* this = (Boss01*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        if (this->actor.colChkInfo.damage != 1) {
            this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        }
        break;

    case 1:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;
        break;

    default:
        break;
    }
}