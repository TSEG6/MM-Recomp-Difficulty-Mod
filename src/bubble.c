#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_bb.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

// doing funnies with home rotation so I can skip doing global variables in case you were curious

void EnBb_SetupFlyIdle(EnBb* this);
void EnBb_Attack(EnBb* this, PlayState* play);
void EnBb_Down(EnBb* this, PlayState* play);

void EnBb_CheckForWall(EnBb* this) {
    s16 yawDiff;

    if (this->actor.bgCheckFlags & BGCHECKFLAG_WALL) {
        yawDiff = this->actor.shape.rot.y - this->actor.wallYaw;
        if (ABS_ALT(yawDiff) > 0x4000) {
            this->actor.shape.rot.y = ((this->actor.wallYaw * 2) - this->actor.shape.rot.y) - 0x8000;
        }

        this->targetYRotation = this->actor.shape.rot.y;
        this->actor.bgCheckFlags &= ~BGCHECKFLAG_WALL;
    }
}

void EnBb_UpdateStateForFlying(EnBb* this) {
    SkelAnime_Update(&this->skelAnime);
    if (this->actor.floorHeight > BGCHECK_Y_MIN) {
        Math_StepToF(&this->actor.world.pos.y, this->actor.floorHeight + this->flyHeightMod, 0.5f);
    }

    this->actor.world.pos.y += Math_CosS(this->bobPhase);
    this->bobPhase += 0x826;
    Math_StepToF(&this->flameScaleY, 0.8f, 0.1f);
    Math_StepToF(&this->flameScaleX, 1.0f, 0.1f);
    EnBb_CheckForWall(this);
    Math_StepToF(&this->actor.speed, this->maxSpeed, 0.5f);
    Math_ApproachS(&this->actor.shape.rot.y, this->targetYRotation, 5, 0x3E8);
    this->actor.world.rot.y = this->actor.shape.rot.y;
}

RECOMP_HOOK_RETURN("EnBb_Init") void BBBuff(Actor* thisx, PlayState* play) {
    EnBb* this = (EnBb*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    s16 baseHealth = this->actor.colChkInfo.health;

    switch (Difficulty) {
    case 0:
        this->attackRange = 400.0f;
        break;

    case 1:
        this->attackRange = 800.0f;
        break;

    default:
        break;
    }

}

RECOMP_HOOK("EnBb_SetupRevive") void Revival(EnBb* this) {
    int Difficulty = (int)recomp_get_config_double("diff_option");

    this->actor.home.rot.y |= 1;
    this->actor.home.rot.y |= 2;

    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.health = this->actor.home.rot.z * 2;
        break;

    case 1:
        this->actor.colChkInfo.health = this->actor.home.rot.z * 3;
        break;

    default:
        this->actor.colChkInfo.health = this->actor.home.rot.z;
        break;
    }
}

RECOMP_HOOK("EnBb_Update") void Updating(Actor* thisx, PlayState* play) {
    EnBb* this = (EnBb*)thisx;
    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 1) {
        if (Rand_ZeroOne() < 0.05f) {
            this->actor.home.pos.x = this->actor.world.pos.x + Rand_CenteredFloat(300.0f);
            this->actor.home.pos.z = this->actor.world.pos.z + Rand_CenteredFloat(300.0f);
            this->actor.home.pos.y = this->actor.world.pos.y + Rand_CenteredFloat(50.0f);
        }
    }

    if (this->actor.home.rot.y & 1) {
        this->actor.home.rot.x++;

        if (this->actor.home.rot.x >= 5) {
            switch (Difficulty) {
            case 1:
                this->attackRange = 350.0f;
                this->actor.colChkInfo.health = this->actor.home.rot.z * 2;
                break;

            case 2:
                this->attackRange = 500.0f;
                this->actor.colChkInfo.health = this->actor.home.rot.z * 3;
                break;

            default:
                break;
            }

            this->actor.home.rot.x = 0;
            this->actor.home.rot.y &= ~1;
        }
    }
    switch (Difficulty) {
    case 1:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        break;

    case 2:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 4;
        break;

    default:
        break;
    }
}

RECOMP_HOOK("EnBb_SetupWaitForRevive") void Vive(EnBb* this) {
    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->timer = 100;
        break;

    case 1:
        this->timer = 50;
        break;

    default:
        this->timer = 200;
        break;
    }
}

RECOMP_HOOK("EnBb_UpdateDamage") void damagestuff(EnBb* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");
    int newJinx = 1200;

    switch (Difficulty) {
    case 0:
        newJinx = 1400;
        break;

    case 1:
        newJinx = 1800;
        break;

    default:
        newJinx = 1200;
        break;
    }

    if (gSaveContext.jinxTimer == 1200) {
        gSaveContext.jinxTimer = newJinx;
    }
}

RECOMP_PATCH void EnBb_SetupAttack(EnBb* this) {
    int Difficulty = (int)recomp_get_config_double("diff_option");
    float diffbasedMaxSpeed = 1.0f;

    switch (Difficulty) {
    case 0:
        diffbasedMaxSpeed = 1.25f;
        break;

    case 1:
        diffbasedMaxSpeed = 1.5f;
        break;

    default:
        diffbasedMaxSpeed = 1.0f;
        break;
    }

    Animation_PlayLoop(&this->skelAnime, &gBubbleAttackAnim);
    this->timer = (s32)Rand_ZeroFloat(20.0f) + 60 * diffbasedMaxSpeed;
    this->flyHeightMod = (Math_CosS(this->bobPhase) * 10.0f) + 30.0f;
    this->targetYRotation = this->actor.yawTowardsPlayer;
    this->maxSpeed = Rand_ZeroFloat(1.5f) + 4.0f * diffbasedMaxSpeed;
    this->actionFunc = EnBb_Attack;
}

RECOMP_PATCH void EnBb_Attack(EnBb* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");
    float sightDist = 120.f;
    float chaseDist = 400.0f;

    switch (Difficulty) {
    case 0:
        sightDist = 240.0f;
        chaseDist = 600.0f;
        break;

    case 1:
        sightDist = 360.0f;
        chaseDist = 800.0f;
        break;

    default:
        sightDist = 120.0f;
        chaseDist = 400.0f;
        break;
    }

    this->targetYRotation = this->actor.yawTowardsPlayer;
    EnBb_UpdateStateForFlying(this);

    if (Animation_OnFrame(&this->skelAnime, 0.0f) || Animation_OnFrame(&this->skelAnime, 5.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_BUBLE_MOUTH);
    }
    else if (Animation_OnFrame(&this->skelAnime, 2.0f) || Animation_OnFrame(&this->skelAnime, 7.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_BUBLE_WING);
    }
    else if (Animation_OnFrame(&this->skelAnime, 0.0f) && (Rand_ZeroOne() < 0.1f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_BUBLE_LAUGH);
    }

    this->timer--;

    if (((this->attackRange + sightDist) < this->actor.xzDistToPlayer) || (this->timer == 0) ||
        (Player_GetMask(play) == PLAYER_MASK_STONE) ||
        (Actor_WorldDistXZToPoint(&this->actor, &this->actor.home.pos) > (chaseDist * 2))) {
        EnBb_SetupFlyIdle(this);
    }
}

RECOMP_PATCH void EnBb_FlyIdle(EnBb* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");
    float sightDist = 120.f;

    switch (Difficulty) {
    case 0:
        sightDist = 240.0f;
        break;

    case 1:
        sightDist = 360.0f;
        break;

    default:
        sightDist = 120.0f;
        break;
    }

    EnBb_UpdateStateForFlying(this);

    if (Animation_OnFrame(&this->skelAnime, 5.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_BUBLE_WING);
    }
    else if (Animation_OnFrame(&this->skelAnime, 0.0f) && (Rand_ZeroOne() < 0.1f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_BUBLE_LAUGH);
    }

    DECR(this->attackWaitTimer);
    this->timer--;

    if ((this->attackWaitTimer == 0) && (this->actor.xzDistToPlayer < this->attackRange + sightDist) &&
        (Player_GetMask(play) != PLAYER_MASK_STONE)) {
        EnBb_SetupAttack(this);
    }
    else if (this->timer == 0) {
        if (Difficulty == 0) EnBb_SetupFlyIdle(this);
    }
}