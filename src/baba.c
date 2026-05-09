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

    switch (Difficulty) {
    case 1:
        this->actor.colChkInfo.health = baseHealth * 2;
        break;

    case 2:
        this->actor.params = DEKUBABA_BIG;
        this->actor.colChkInfo.health = baseHealth * 6;
        break;

    default:
        break;
    }
}

// backup health thing because sometimes it doesn't apply
RECOMP_HOOK_RETURN("EnDekubaba_Init") void InitChange2(Actor* thisx, PlayState* play) {

    EnDekubaba* this = (EnDekubaba*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    u8 baseHealth = this->actor.colChkInfo.health;

    switch (Difficulty) {
    case 1:
        this->actor.colChkInfo.health = baseHealth * 1.25;
        break;

    case 2:
        this->actor.colChkInfo.health = baseHealth * 1.5;
        break;

    default:
        break;
    }
}