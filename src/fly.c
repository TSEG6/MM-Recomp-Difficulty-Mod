#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_grasshopper.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

void EnGrasshopper_SetupApproachPlayer(EnGrasshopper* this, PlayState* play);
void EnGrasshopper_Attack(EnGrasshopper* this, PlayState* play);

RECOMP_HOOK_RETURN("EnGrasshopper_Init") void DFlyHealthBoost(Actor* thisx, PlayState* play) {

	EnGrasshopper* this = (EnGrasshopper*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    u8 baseHealth = this->actor.colChkInfo.health;

    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.health = baseHealth * 3;
        break;

    case 1: {
        this->actor.colChkInfo.health = baseHealth * 6;
        break;
    }
    default:
        break;
    }
}

RECOMP_HOOK("EnGrasshopper_Update") void DFlyUpdate(Actor* thisx, PlayState* play) {

    EnGrasshopper* this = (EnGrasshopper*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");


        switch (Difficulty) {
        case 0:
            if (this->actor.colChkInfo.health != 0 && this->actionFunc == EnGrasshopper_Attack) this->skelAnime.playSpeed = 0.75;
            break;

        case 1:
            if (this->actor.colChkInfo.health != 0 && this->actionFunc == EnGrasshopper_Attack) this->skelAnime.playSpeed = 0.5;
            break;

        default:
            break;
        }
}

RECOMP_HOOK("EnGrasshopper_SetupAttack") void SpeedupAttackDFly(EnGrasshopper* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");


    switch (Difficulty) {
    case 0:
        this->actor.speed = 7.0f;
        break;

    case 1:
        this->actor.speed = 9.0f;
        break;

    default:
        break;
    }
}

RECOMP_HOOK("EnGrasshopper_ApproachPlayer") void DFlyApproachSpeed(EnGrasshopper* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");


    switch (Difficulty) {
    case 0:
        Math_ApproachF(&this->approachSpeed, 11.0f, 1.5f, 3.0f);
        break;

    case 1:
        Math_ApproachF(&this->approachSpeed, 14.0f, 2.0f, 4.0f);
        break;

    default:
        break;
    }
}

RECOMP_HOOK("EnGrasshopper_Fly") void DFlyFlying(EnGrasshopper* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float Distance = 200.0f;

    switch (Difficulty) {
    case 0:
        Distance = 350.0f;
        break;

    case 1:
        Distance = 500.0f;
        break;

    default:
        break;
    }

    if ((Player_GetMask(play) != PLAYER_MASK_STONE) && !CHECK_EVENTINF(EVENTINF_41) && !this->shouldTurn &&
        (this->actor.xzDistToPlayer < Distance)) {
        EnGrasshopper_SetupApproachPlayer(this, play);

    }

}