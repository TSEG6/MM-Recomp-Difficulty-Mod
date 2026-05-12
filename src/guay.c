#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_crow.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"


RECOMP_HOOK_RETURN("EnCrow_SetupDiveAttack") void AttackGuay(EnCrow* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");


    switch (Difficulty) {
    case 0:
        this->timer = 500;
        this->actor.speed = 6.0f;
        break;

    case 1:
        this->timer = 700;
        this->actor.speed = 8.0f;
        break;

    default:
        break;
    }
}