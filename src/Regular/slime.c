#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "gameplay_keep.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "z_en_slime.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

// doing funnies with home rotation so I can skip doing global variables in case you were curious

#define ICE_BLOCK_TIMER_MAX 254
#define ICE_BLOCK_UNUSED (ICE_BLOCK_TIMER_MAX + 1)

void EnSlime_Damaged(EnSlime* this, PlayState* play);
void EnSlime_Revive(EnSlime* this, PlayState* play);
void EnSlime_SetupTurnToPlayer(EnSlime* this);
void EnSlime_SetupJump(EnSlime* this);
void ReviveBuffFix(EnSlime* this);
void EnSlime_Jump(EnSlime* this, PlayState* play);
void EnSlime_SetupMoveInDirection(EnSlime* this);

extern Gfx gItemDropDL[];

AnimatedMaterial* sSlimeTexAnim;

static TexturePtr sEyeTextures[] = {
    gChuchuEyeOpenTex,
    gChuchuEyeHalfTex,
    gChuchuEyeClosedTex,
    gChuchuEyeHalfTex,
};

static Vec3f sBodyPartPosOffsets[EN_SLIME_BODYPART_MAX] = {
    { 2000.0f, 2000.0f, 0.0f },     // EN_SLIME_BODYPART_0
    { -1500.0f, 2500.0f, -500.0f }, // EN_SLIME_BODYPART_1
    { -500.0f, 1000.0f, 2500.0f },  // EN_SLIME_BODYPART_2
    { 0.0f, 4000.0f, 0.0f },        // EN_SLIME_BODYPART_3
    { 0.0f, 2000.0f, -2000.0f },    // EN_SLIME_BODYPART_4
};

typedef enum EnSlimeEyeTexture {
    /* 0 */ EN_SLIME_EYETEX_OPEN,
    /* 1 */ EN_SLIME_EYETEX_HALF,
    /* 2 */ EN_SLIME_EYETEX_CLOSED,
    /* 3 */ EN_SLIME_EYETEX_HALF2,
    /* 4 */ EN_SLIME_EYETEX_MAX
} EnSlimeEyeTexture;

void EnSlime_Blink(EnSlime* this) {
    if (this->eyeTexIndex != EN_SLIME_EYETEX_OPEN) {
        this->eyeTexIndex++;
        if (this->eyeTexIndex == EN_SLIME_EYETEX_MAX) {
            this->eyeTexIndex = EN_SLIME_EYETEX_OPEN;
        }
    }
    else if (Rand_ZeroOne() < 0.05f) {
        this->eyeTexIndex = EN_SLIME_EYETEX_HALF;
    }
}

RECOMP_HOOK_RETURN("EnSlime_Init")
void SlimeBuff(Actor* thisx, PlayState* play) {
    EnSlime* this = (EnSlime*)thisx;
    s32 reviveTimeSeconds;
    float Distance = 30000.0f;
    int Difficulty = (int)recomp_get_config_double("diff_option");

    this->actor.home.rot.x = 0;
    this->actor.home.rot.y = 0;
    this->actor.home.rot.z = this->actor.colChkInfo.health;

    switch (Difficulty) {
    case 0: {
        Distance = 60000.0f;
        reviveTimeSeconds = 10;
        this->reviveTime = (reviveTimeSeconds * 10) + 100;
        this->actor.colChkInfo.health = this->actor.home.rot.z * 2;
        break;
    }

    case 1: {
        Distance = 90000.0f;
        reviveTimeSeconds = 5;
        this->reviveTime = (reviveTimeSeconds * 5);
        this->actor.colChkInfo.health = this->actor.home.rot.z * 5;
        break;
    }

    default: {
        Distance = 30000.0f;
        this->actor.colChkInfo.health = this->actor.home.rot.z;
        break;
    }
    }

    if (this->actor.shape.rot.x <= 0) {
        this->distLimit = Distance;
    }
    else {
        this->distLimit = this->actor.shape.rot.x * 40.0f;
    }
}

RECOMP_HOOK("EnSlime_SetupRevive") void ReviveBuff(EnSlime* this) {
    this->actor.home.rot.y = 1;
}

RECOMP_HOOK("EnSlime_Revive") void ReviveBuff2(EnSlime* this) {
    this->actor.home.rot.y = 1;
}

RECOMP_HOOK("EnSlime_Update") void ReviveBuffFix(EnSlime* this) {
    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (this->actor.home.rot.y) {
        this->actor.home.rot.x++;

        if (this->actor.home.rot.x >= 2) {
            if (this->actor.colChkInfo.health > 0) {
                switch (Difficulty) {
                case 0:
                    this->actor.colChkInfo.health = this->actor.home.rot.z * 2;
                    break;

                case 1:
                    this->actor.colChkInfo.health = this->actor.home.rot.z * 5;
                    break;

                default:
                    break;
                }
                this->actor.home.rot.x = 0;
                this->actor.home.rot.y = 0;
            }
        }
    }

    if (Difficulty == 2) {
        if (Rand_ZeroOne() < 0.05f) {
            this->actor.home.pos.x =
                this->actor.world.pos.x + Rand_CenteredFloat(300.0f);

            this->actor.home.pos.z =
                this->actor.world.pos.z + Rand_CenteredFloat(300.0f);

            this->actor.home.pos.y =
                this->actor.world.pos.y + Rand_CenteredFloat(50.0f);
        }
    }
}

RECOMP_HOOK_RETURN("EnSlime_MoveInDirection") void SlimeMovement(EnSlime* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");
    f32 sinFactor;

    sinFactor = fabsf(Math_SinF(this->timer * (M_PIf / 24)));
    switch (Difficulty) {
    case 0:
        this->actor.speed = (0.8f * sinFactor) + 0.2f * 1.5;
        break;

    case 1:
        this->actor.speed = (0.8f * sinFactor) + 0.2f * 3;
        break;

    default:
        break;
    }
}

RECOMP_PATCH void EnSlime_TurnToPlayer(EnSlime* this, PlayState* play) {
    f32 factorY;
    f32 scaleXZ;
    Player* player = GET_PLAYER(play);
    Vec3f predictedPos;
    s16 targetYaw;
    f32 dx, dz, distance, travelTime, predictFactor;

    f32 JumpSpeed;
    f32 multiplier = 1.0f;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    dx = player->actor.world.pos.x - this->actor.world.pos.x;
    dz = player->actor.world.pos.z - this->actor.world.pos.z;
    distance = sqrtf(SQ(dx) + SQ(dz));

    switch (Difficulty) {
    case 0:
        multiplier = 2.0f;
        break;
    case 1:
        multiplier = 3.0f;
        break;
    default:
        multiplier = 1.0f;
        break;
    }

    if (distance < 60.0f) {
        JumpSpeed = 2.5f * multiplier / 2.0f;
    }
    else if (distance < 120.0f) {
        JumpSpeed = 4.5f * multiplier / 2.0f;
    }
    else {
        JumpSpeed = 7.0f * multiplier / 2.0f;
    }

    if (JumpSpeed == 0.0f) {
        JumpSpeed = 1.0f;
    }

    travelTime = distance / JumpSpeed;
    travelTime = CLAMP(travelTime, 0.0f, 15.0f);

    if (Difficulty == 1) {
        predictFactor = 0.85f;
    }
    else {
        predictFactor = 0.45f;
    }

    predictedPos.x = player->actor.world.pos.x + (player->actor.velocity.x * travelTime * predictFactor);
    predictedPos.y = player->actor.world.pos.y;
    predictedPos.z = player->actor.world.pos.z + (player->actor.velocity.z * travelTime * predictFactor);

    targetYaw = Math_Vec3f_Yaw(&this->actor.world.pos, &predictedPos);

    this->timer--;
    EnSlime_Blink(this);
    Math_ApproachS(&this->actor.shape.rot.y, targetYaw, 4, 0x1000);

    if (this->timer >= 0) {
        factorY = 8 - this->timer;
        scaleXZ = ((factorY * 0.04f) + 1.0f) * 0.01f;
        this->actor.scale.x = scaleXZ;
        this->actor.scale.y = (1.0f - (factorY * 0.05f)) * 0.01f;
        this->actor.scale.z = scaleXZ;
    }
    if (this->timer < -2) {
        EnSlime_SetupJump(this);
    }
}

RECOMP_PATCH void EnSlime_SetupJump(EnSlime* this) {
    int Difficulty = (int)recomp_get_config_double("diff_option");
    float dist = this->actor.xzDistToPlayer;
    float Multiplier = 1;

    switch (Difficulty) {
    case 0:
        Multiplier = 2;
        break;

    case 1:
        Multiplier = 3;
        break;

    default:
        Multiplier = 1;
        break;
    }

    if (dist < 60.0f) {
        if (Difficulty != 0) {
            this->actor.velocity.y = 6.0f;
            this->actor.speed = 2.5f * Multiplier / 2;
            this->actor.gravity = -1.0f;
        }
    }
    else if (dist < 120.0f) {
        this->actor.velocity.y = 11.0f;
        this->actor.speed = 4.5f * Multiplier / 2;
        this->actor.gravity = -2.0f;
    }
    else {
        this->actor.velocity.y = 18.0f;
        this->actor.speed = 7.0f * Multiplier / 2;
        this->actor.gravity = -3.5f;
    }
    this->actor.world.rot.y = this->actor.shape.rot.y;
    this->timer = 12;
    this->eyeTexIndex = EN_SLIME_EYETEX_OPEN;
    Math_StepToF(&this->actor.scale.x, 0.008f, 0.0025f);
    Math_StepToF(&this->actor.scale.y, 0.011f, 0.0025f);
    this->actor.scale.z = this->actor.scale.x;
    Actor_PlaySfx(&this->actor, NA_SE_EN_SLIME_JUMP);
    this->actionFunc = EnSlime_Jump;
}

RECOMP_HOOK("EnSlime_Idle") void Sight(EnSlime* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");
    float Distance = 280.0f;

    switch (Difficulty) {
    case 0:
        Distance = 480.f;
        break;

    case 1:
        Distance = 690.f;
        break;

    default:
        Distance = 280.0f;
        break;
    }

    if ((Player_GetMask(play) != PLAYER_MASK_STONE) && (this->actor.xzDistToPlayer < Distance) &&
        Actor_IsFacingPlayer(&this->actor, 0x5000)) {
        EnSlime_SetupTurnToPlayer(this);
    }
    else if (this->timer == 0) {
        // Start moving (remaining idle)
        EnSlime_SetupMoveInDirection(this);
    }
}