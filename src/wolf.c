#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_wf.h"
#include "overlays/actors/ovl_En_Bom_Chu/z_en_bom_chu.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "overlays/actors/ovl_Obj_Ice_Poly/z_obj_ice_poly.h"

void func_80991948(EnWf* this);
void func_8099223C(EnWf* this);
void func_809922B4(EnWf* this, PlayState* play);
void func_80992068(EnWf* this, PlayState* play);


RECOMP_HOOK_RETURN("EnWf_Init") void HealthWolf(Actor* thisx, PlayState* play) {

    EnWf* this = (EnWf*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.health = 9;
        break;

    case 1:
        this->actor.colChkInfo.health = 12;
        break;

    default:
        break;
    }

}


RECOMP_HOOK("EnWf_Update") void WolfUpdate(Actor* thisx, PlayState* play) {
    EnWf* this = (EnWf*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMultiplier = 1.0f;

    switch (Difficulty) {
    case 0:
        speedMultiplier = 1.25f;
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        break;

    case 1:
        speedMultiplier = 1.5f;
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;
        break;

    default:
        break;
    }

    if (this->actor.colChkInfo.health != 0 && this->actionFunc != func_809922B4) {
        this->skelAnime.playSpeed = speedMultiplier;
    }

    if (this->actor.colChkInfo.health == 0) this->actor.speed = 0;

    if (this->actor.speed <= 6.0f && this->actionFunc != func_809922B4) {
        this->actor.speed *= speedMultiplier;
    }
}


RECOMP_HOOK("func_809928CC") void LessTimer2(EnWf* this, PlayState* play) {

    Player* player = GET_PLAYER(play);

    int Difficulty = (int)recomp_get_config_double("diff_option");
    int Timer = 10;

    switch (Difficulty) {
    case 0:
        Timer = 6;
        break;

    case 1:
        Timer = 3;
        break;

    default:
        break;
    }


    if (SkelAnime_Update(&this->skelAnime)) {
        if (this->unk_2A0 != 0) {
            this->unk_2A0--;
        }
        else if (func_800BE184(play, &this->actor, 100.0f, 10000, 0x4000, this->actor.shape.rot.y)) {
            if ((player->meleeWeaponAnimation != PLAYER_MWA_JUMPSLASH_START) || ((play->gameplayFrames % 2) != 0)) {
                this->unk_2A0 = Timer;
            }
            else {
                func_8099223C(this);
            }
        }
    }
}