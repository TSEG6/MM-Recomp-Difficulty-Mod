#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_fz.h"
#include "attributes.h"
#include "overlays/actors/ovl_En_Wiz/z_en_wiz.h"
#include "assets/objects/object_fz/object_fz.h"
#include "assets/objects/gameplay_keep/gameplay_keep.h"

RECOMP_HOOK("EnFz_Update") void FzUpdate(Actor* thisx, PlayState* play) {

    EnFz* this = (EnFz*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        break;

    case 1:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;
        break;

    default:
        break;
    }
}