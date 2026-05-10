#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_rat.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "overlays/actors/ovl_En_Bom/z_en_bom.h"

float atkRange;

void EnRat_SetupSpottedPlayer(EnRat* this);

RECOMP_HOOK("EnRat_Init") void RatStart(Actor* thisx, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        atkRange = 550.0f;
        break;

    case 1:
        atkRange = 750.0f;
        break;

    default:
        break;
    }

}


RECOMP_HOOK("EnRat_Update") void RatUpdate(Actor* thisx, PlayState* play) {

    EnRat* this = (EnRat*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 1) {

        if (Rand_ZeroOne() < 0.02f) {

            this->actor.home.pos.x = this->actor.world.pos.x + Rand_CenteredFloat(300.0f);
            this->actor.home.pos.z = this->actor.world.pos.z + Rand_CenteredFloat(300.0f);
        }
    }
}

RECOMP_HOOK("EnRat_Idle") void RatIdle(Actor* thisx, PlayState* play) {

    EnRat* this = (EnRat*)thisx;
    Player* player = GET_PLAYER(play);

    int Difficulty = (int)recomp_get_config_double("diff_option");


    switch (Difficulty) {
    case 0:
        this->actor.speed = 2.0f;
        this->skelAnime.playSpeed = 1.1f;
        break;

    case 1:
        this->actor.speed = 4.0f;
        this->skelAnime.playSpeed = 1.25f;
        break;

    default:
        this->actor.speed = 2.0f;
        this->skelAnime.playSpeed = 1.0f;
        break;
    }

    if (!(player->stateFlags3 & PLAYER_STATE3_100) && (this->actor.xzDistToPlayer < atkRange) &&
        (Player_GetMask(play) != PLAYER_MASK_STONE) && Actor_IsFacingPlayer(&this->actor, 0x3800)) {
        EnRat_SetupSpottedPlayer(this);
    }

}

RECOMP_HOOK("EnRat_ChasePlayer") void RatChase(EnRat* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->skelAnime.playSpeed = 1.5f;
        break;

    case 1:
        this->skelAnime.playSpeed = 2.0f;
        break;

    default:
        break;
    }
}

RECOMP_HOOK_RETURN("EnRat_ChasePlayer") void RatChase2(EnRat* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.speed = 8.2f;
        break;

    case 1:
        this->actor.speed = 10.4f;
        break;

    default:
        break;
    }
}