#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_pp.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

void EnPp_Roar(EnPp* this, PlayState* play);
void EnPp_SetupCharge(EnPp* this);

typedef enum {
    /*  0 */ EN_PP_ACTION_IDLE,
    /*  1 */ EN_PP_ACTION_CHARGE,
    /*  2 */ EN_PP_ACTION_ATTACK,
    /*  3 */ EN_PP_ACTION_BOUNCED,
    /*  4 */ EN_PP_ACTION_ROAR,
    /*  5 */ EN_PP_ACTION_JUMP,
    /*  6 */ EN_PP_ACTION_STUNNED_OR_FROZEN,
    /*  7 */ EN_PP_ACTION_MASK_DETACH,
    /*  8 */ EN_PP_ACTION_DAMAGED,
    /*  9 */ EN_PP_ACTION_DEAD,
    /* 10 */ EN_PP_ACTION_MASK_DEAD,
    /* 11 */ EN_PP_ACTION_SPAWN_BODY_PARTS,
    /* 12 */ EN_PP_ACTION_DONE_SPAWNING_BODY_PARTS,
    /* 13 */ EN_PP_ACTION_BODY_PART_MOVE
} EnPpAction;

typedef enum {
    /*  0 */ EN_PP_ANIM_IDLE,
    /*  1 */ EN_PP_ANIM_WALK,
    /*  2 */ EN_PP_ANIM_WIND_UP,
    /*  3 */ EN_PP_ANIM_CHARGE,
    /*  4 */ EN_PP_ANIM_ATTACK,
    /*  5 */ EN_PP_ANIM_DAMAGE,
    /*  6 */ EN_PP_ANIM_ROAR,
    /*  7 */ EN_PP_ANIM_TURN_TO_FACE_PLAYER,
    /*  8 */ EN_PP_ANIM_JUMP,
    /*  9 */ EN_PP_ANIM_LAND,
    /* 10 */ EN_PP_ANIM_MAX
} EnPpAnimation;

extern void EnPp_ChangeAnim(EnPp* this, s32 animIndex);

// Defense
RECOMP_HOOK("EnPp_Update") void LoopUpdate(Actor* thisx, PlayState* play) {

    EnPp* this = (EnPp*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");

        switch (Difficulty) {
        case 0:
            if (this->actor.colChkInfo.damage != 1) {
                this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
            }
            break;

        case 1: {
            this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;
            break;
        }
        default:
            break;
        }
}

// Increases attack range
RECOMP_HOOK("EnPp_Idle") void LoopIdle(EnPp* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->attackRange = 300.0f;
        break;

    case 1: {
            this->attackRange = 400.0f;
        break;
    }
    default:
        break;
    }
}

RECOMP_PATCH void EnPp_SetupRoar(EnPp* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->timer = 10;
        break;
    case 1:
        EnPp_SetupCharge(this);
        break;
    default:
        break;
    }

    this->secondaryTimer = 0;
    this->chargeAndBounceSpeed = 16.0f;
    EnPp_ChangeAnim(this, EN_PP_ANIM_ROAR);
    this->action = EN_PP_ACTION_ROAR;
    this->actionFunc = EnPp_Roar;
}

// Speed Increases
RECOMP_HOOK("EnPp_Charge") void ChargeSpeedUp(EnPp* this, PlayState* play) {
    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMult = 1.0f;

    switch (Difficulty) {
    case 0:
        speedMult = 1.5f;
        break;
    case 1:
        speedMult = 2.0f;
        break;
    default:
        break;
    }

    if (this->animIndex == EN_PP_ANIM_CHARGE) {

        if (!this->chargesInStraightLines) {
            Math_ApproachF(&this->actor.speed, 10.0f * speedMult, 0.3f, 1.0f * speedMult);
        }
        else {
            if (this->timer == 20) {
                this->chargeAndBounceSpeed = 14.0f * speedMult;
            }
            if (this->timer < 10) {
                Math_ApproachZeroF(&this->chargeAndBounceSpeed, 0.3f, 0.2f * speedMult);
            }
        }
    }
}