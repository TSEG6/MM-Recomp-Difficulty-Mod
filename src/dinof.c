#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_dinofos.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"

void EnDinofos_PlayCutscene(EnDinofos* this, PlayState* play);
void EnDinofos_SetupCircleAroundPlayer(EnDinofos* this, PlayState* play);
void EnDinofos_DodgeProjectile(EnDinofos* this, PlayState* play);
void EnDinofos_Walk(EnDinofos* this, PlayState* play);
void EnDinofos_SetupWalk(EnDinofos* this, PlayState* play);
void EnDinofos_SetupBreatheFire(EnDinofos* this, PlayState* play);
void EnDinofos_SetupSlash(EnDinofos* this);
void EnDinofos_SetupTurnToPlayer(EnDinofos* this);
void EnDinofos_IntroCutsceneYell(EnDinofos* this, PlayState* play);
void EnDinofos_JumpSlash(EnDinofos* this, PlayState* play);
void EnDinofos_Jump(EnDinofos* this, PlayState* play);
void EnDinofos_SetupChooseJump(EnDinofos* this, s32 jumpType);
void EnDinofos_SetupIdle(EnDinofos* this);
void EnDinofos_SetupStartBreatheFire(EnDinofos* this);

s32 EnDinofos_IsFacingPlayer(EnDinofos* this) {
    s16 angleToPlayer = (this->actor.yawTowardsPlayer - this->headRotY) - this->actor.shape.rot.y;

    if (ABS_ALT(angleToPlayer) < 0x3000) {
        return true;
    }
    return false;
}

RECOMP_PATCH void EnDinofos_ChooseAction(EnDinofos* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float dist = this->actor.xzDistToPlayer;
    float randVal = Rand_ZeroOne();
    float slashChance = 0.4f;
    float fireChance = 0.2f;
    float circleChance = 0.6f;
    float jumpChance = 0.1f;

    if (Difficulty >= 1) {
        slashChance = 0.8f;
        fireChance = 0.4f;
        circleChance = 0.2f;
        jumpChance = 0.5f;
    }

    if (EnDinofos_IsFacingPlayer(this)) {
        if (dist < 120.0f) {
            if (!Actor_OtherIsTargeted(play, &this->actor) && (randVal < slashChance)) {
                EnDinofos_SetupSlash(this);
            }
            else if (Rand_ZeroOne() < fireChance) {
                EnDinofos_SetupBreatheFire(this, play);
            }
            else if (Rand_ZeroOne() < circleChance) {
                EnDinofos_SetupCircleAroundPlayer(this, play);
            }
            else {
                EnDinofos_SetupWalk(this, play);
            }
        }
        else if (dist < 300.0f) {
            if (randVal < jumpChance) {
                EnDinofos_SetupChooseJump(this, DINOFOS_JUMP_TYPE_FORWARD);
            }
            else if (Rand_ZeroOne() < fireChance) {
                EnDinofos_SetupBreatheFire(this, play);
            }
            else if (Rand_ZeroOne() < circleChance) {
                EnDinofos_SetupCircleAroundPlayer(this, play);
            }
            else {
                EnDinofos_SetupWalk(this, play);
            }
        }
        else {
            EnDinofos_SetupWalk(this, play);
        }
    }
    else {
        float turnChance = (Difficulty >= 1) ? 0.9f : 0.6f;

        if (Rand_ZeroOne() < turnChance) {
            EnDinofos_SetupTurnToPlayer(this);
        }
        else {
            EnDinofos_SetupIdle(this);
        }
    }
}

RECOMP_HOOK("EnDinofos_Walk") void WalkUpdateDinof(EnDinofos* this, PlayState* play) {

    f32 targetDist;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Actor_OtherIsTargeted(play, &this->actor)) {
        targetDist = 170.0f;
    }
    else {
        targetDist = 70.0f;
    }

    switch (Difficulty) {
    case 0:
        if (this->actor.xzDistToPlayer <= targetDist) {
            Math_StepToF(&this->actor.speed, -9.0f, 0.75f);
        }
        else {
            Math_StepToF(&this->actor.speed, 9.0f, 0.75f);
        }
        break;

    case 1: {
        if (this->actor.xzDistToPlayer <= targetDist) {
            Math_StepToF(&this->actor.speed, -12.0f, 1.5f);
        }
        else {
            Math_StepToF(&this->actor.speed, 12.0f, 1.5f);
        }
        break;
    }
    default:
        break;
    }
}

RECOMP_HOOK("EnDinofos_Update") void DinoFUpdate(Actor* thisx, PlayState* play2) {

    EnDinofos* this = (EnDinofos*)thisx;
    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        if (this->actor.colChkInfo.health != 0) this->skelAnime.playSpeed = 1.5f;
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 3;
        break;

    case 1: {
        if (this->actor.colChkInfo.health != 0) this->skelAnime.playSpeed = 2.0f;
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 5;
        break;
    }
    default:
        break;
    }


}