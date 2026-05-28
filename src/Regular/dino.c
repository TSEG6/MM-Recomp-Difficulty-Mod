#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_dodongo.h"
#include "overlays/actors/ovl_En_Bom/z_en_bom.h"
#include "overlays/actors/ovl_En_Bombf/z_en_bombf.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "overlays/effects/ovl_Effect_Ss_Hitmark/z_eff_ss_hitmark.h"

// me when you use flags here but weird home rotation stuff elsewhere (it's funny)

void func_80877E60(EnDodongo* this, PlayState* play);

#define Health_Buffed 0x20000000

RECOMP_HOOK("EnDodongo_Init") void InitDino(Actor* thisx, PlayState* play) {
    EnDodongo* this = (EnDodongo*)thisx;

    this->actor.flags &= ~Health_Buffed;
}

RECOMP_HOOK("EnDodongo_Update") void DinoUpdate(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    EnDodongo* this = (EnDodongo*)thisx;
    int difficulty = (int)recomp_get_config_double("diff_option");
    bool use_speed = true;

    if (!(this->actor.flags & Health_Buffed)) {
        if (difficulty == 1) {
            this->actor.colChkInfo.health *= 2;
        }
        else if (difficulty == 2) {
            this->actor.colChkInfo.health *= 4;
        }
        this->actor.flags |= Health_Buffed;
    }

    if (this->actor.colChkInfo.health <= 0) {
        use_speed = false;
    }
    else {
        use_speed = true;
    }

    if (use_speed) {
        switch (difficulty) {
        case 0:
            this->skelAnime.playSpeed = 1.5f;
            break;
        case 1:
            this->skelAnime.playSpeed = 2.0f;
            break;
        default:
            this->skelAnime.playSpeed = 1.0f;
            break;
        }
    }
    else {
        this->skelAnime.playSpeed = 1.0f;
    }
}

RECOMP_HOOK("func_80877494") void MovementSpeed(EnDodongo* this) {
    int difficulty = (int)recomp_get_config_double("diff_option");

    switch (difficulty) {
    case 0:
        this->actor.speed = this->unk_334 * 4.5f;
        break;
    case 1:
        this->actor.speed = this->unk_334 * 6.5f;
        break;
    default:
        this->actor.speed = this->unk_334 * 1.5f;
        break;
    }
}

RECOMP_HOOK("func_80877DE0") void NoSpeed(EnDodongo* this) {

    this->skelAnime.playSpeed = 1.0f;
}

RECOMP_PATCH s32 func_80877278(EnDodongo* this, PlayState* play) {

    return false;
}