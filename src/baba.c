#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_dekubaba.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "overlays/effects/ovl_Effect_Ss_Hahen/z_eff_ss_hahen.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

RECOMP_HOOK("EnDekubaba_Init") void InitChange(Actor* thisx, PlayState* play) {
    EnDekubaba* this = (EnDekubaba*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    u8 baseHealth = this->actor.colChkInfo.health;

    if (this->actor.home.rot.z == 0) {
        switch (Difficulty) {
        case 0:
            this->actor.colChkInfo.health = baseHealth * 2;
            break;

        case 1:
            this->actor.params = DEKUBABA_BIG;
            this->actor.colChkInfo.health = baseHealth * 6;
            break;

        default:
            break;
        }
        this->actor.home.rot.z = 1;
    }
}

RECOMP_HOOK_RETURN("EnDekubaba_Init") void InitChange2(Actor* thisx, PlayState* play) {
    EnDekubaba* this = (EnDekubaba*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    u8 baseHealth = this->actor.colChkInfo.health;

    if (this->actor.home.rot.z == 0) {
        switch (Difficulty) {
        case 0:
            this->actor.colChkInfo.health = baseHealth * 2;
            break;

        case 1:
            this->actor.colChkInfo.health = baseHealth * 3;
            break;

        default:
            break;
        }
        this->actor.home.rot.z = 1;
    }
}