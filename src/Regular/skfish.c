#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_pr2.h"
#include "overlays/actors/ovl_En_Encount1/z_en_encount1.h"

// Increases health and speed
RECOMP_HOOK("EnPr2_Update") void nodefensecansaveanenemywithonehp(Actor* thisx, PlayState* play) {

	EnPr2* this = (EnPr2*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMultiplier = 1.0f;

    #define HEALTH_SCALED_FLAG 0x8000

    if (!(this->actor.params & HEALTH_SCALED_FLAG)) {

        switch (Difficulty) {
        case 0:
            this->actor.colChkInfo.health = 2;
            break;

        case 1:
            this->actor.colChkInfo.health = 3;
            break;

        default:
            break;
        }
        this->actor.params |= HEALTH_SCALED_FLAG;
    }

    switch (Difficulty) {
    case 0:
        speedMultiplier = 1.75f;
        break;

    case 1:
        speedMultiplier = 2.5f;
        this->unk_1F4 = 255;
        break;

    default:
        break;
    }

    if (this->actor.colChkInfo.health != 0) {

        if (this->actor.speed <= 6.0f) {
            this->actor.speed *= speedMultiplier;
        }
    }

    if (this->actor.colChkInfo.health == 0) this->actor.speed = 0;
}

// Is on screen (on Hard it's always true)
s32 func_80A7429C(EnPr2* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 screenPosX;
    s16 screenPosY;

    Actor_GetScreenPos(play, &this->actor, &screenPosX, &screenPosY);

    int Difficulty = (int)recomp_get_config_double("diff_option");

    if (Difficulty == 1) {
        return true;
    }

    if ((fabsf(player->actor.world.pos.y - this->actor.world.pos.y) > 160.0f) ||
        (this->actor.projectedPos.z < -40.0f) ||
        (screenPosX < 0) || (screenPosX > SCREEN_WIDTH) ||
        (screenPosY < 0) || (screenPosY > SCREEN_HEIGHT)) {
        return false;
    }

    if (!(player->stateFlags1 & PLAYER_STATE1_8000000)) {
        return false;
    }
    else {
        return true;
    }
}