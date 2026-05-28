#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_pp.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

void EnPp_SetupCharge(EnPp* this);

RECOMP_HOOK("EnPp_Update") void LoopUpdate(Actor* thisx, PlayState* play) {

    EnPp* this = (EnPp*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMultiplier = 1.0f;

        switch (Difficulty) {
        case 0:
            this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
            speedMultiplier = 1.25f;
            break;

        case 1: {
            this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;
            speedMultiplier = 1.5f;
            break;
        }
        default:
            break;
        }

        if (this->actor.colChkInfo.health != 0) {
            this->skelAnime.playSpeed = speedMultiplier;
        }

        if (this->actor.colChkInfo.health != 0 && this->actor.speed <= 6.0f) {
            this->actor.speed *= speedMultiplier;
        }
}

RECOMP_HOOK("EnPp_Idle") void LoopIdle(EnPp* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->attackRange = 300.0f;
        break;

    case 1: {
        this->actionVar.hasDoneFirstRoar = true;
            this->attackRange = 400.0f;
        break;
    }
    default:
        break;
    }
}