#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_dekubaba.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "overlays/effects/ovl_Effect_Ss_Hahen/z_eff_ss_hahen.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"
#include "eztr_api.h"

// doing funnies with home rotation so I can skip doing global variables in case you were curious

bool IsBig = false;
extern bool fixText;

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
        Player* player = GET_PLAYER(play);

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

RECOMP_HOOK("EnDekubaba_Update") void BabaUpdate(Actor* thisx, PlayState* play) {

    EnDekubaba* this = (EnDekubaba*)thisx;
    Player* player = GET_PLAYER(play);

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        if (player->focusActor == &this->actor) IsBig = false;
        break;

    case 1:
        if (player->focusActor == &this->actor) {
            IsBig = true;
            fixText = false;
        }
        break;

    default:
        break;
    }

}

EZTR_MSG_CALLBACK(bbaba_text_callback) {
    if (IsBig) {
        EZTR_MsgSContent_Sprintf(buf->data.content, "|05That's the |00Big Deku Baba|05...|11Don't you already know how|11to deal with them!?|BF");
    }
    if (fixText) {
        EZTR_MsgSContent_Sprintf(buf->data.content, "Bio Deku Baba|05...|11Just calm down and aim carefully!|11And don't blame me if one hit|11doesn't get rid of it!|BF");
    }
}

EZTR_ON_INIT void init_text_bigbaba() {
    EZTR_Basic_ReplaceText(
        0x1908,
        EZTR_BLUE_TEXT_BOX,
        0,
        EZTR_ICON_NO_ICON,
        EZTR_NO_VALUE,
        EZTR_NO_VALUE,
        EZTR_NO_VALUE,
        true,
        "",
        bbaba_text_callback
    );
}