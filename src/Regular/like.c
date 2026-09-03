#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_rr.h"
#include "z64rumble.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "assets/objects/object_rr/object_rr.h"

// Extra defense
RECOMP_HOOK("EnRr_Update") void LLUpdate(Actor* thisx, PlayState* play) {

	EnRr* this = (EnRr*)thisx;

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

// It can move while reaching now
RECOMP_HOOK("func_808FB088") void LLMovewhilereach(EnRr* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.speed = 2.0f;
        break;

    case 1:
        this->actor.speed = 3.0f;
        break;

    default:
        break;
    }

}

// Just makes it go faster.
RECOMP_PATCH void func_808FA238(EnRr* this, f32 arg1) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.speed = 3.0f;
        break;

    case 1:
        this->actor.speed = 4.0f;
        break;

    default:
        break;
    }
    Actor_PlaySfx(&this->actor, NA_SE_EN_LIKE_WALK);
}