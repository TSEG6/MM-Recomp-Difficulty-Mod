#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_bat.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "assets/objects/object_bat/object_bat.h"

#define BAD_BAT_MAX_NUMBER_ATTACKING 9

s32 sNumberAttacking;

void EnBat_SetupDiveAttack(EnBat* this);

RECOMP_HOOK_RETURN("EnBat_SetupDiveAttack") void AttackPlayerBat(EnBat* this) {

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

RECOMP_HOOK("EnBat_FlyIdle") void Flying(EnBat* this, PlayState* play) {

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float AtkDistance = 300.0f;

    switch (Difficulty) {
    case 0:
        AtkDistance = 500.0f;
        break;

    case 1:
        AtkDistance = 800.0f;
        break;

    default:
        break;
    }

    if ((this->actor.xzDistToPlayer < AtkDistance) && (this->timer == 0) && (Player_GetMask(play) != PLAYER_MASK_STONE) &&
        (sNumberAttacking < BAD_BAT_MAX_NUMBER_ATTACKING) &&
        (!(this->paramFlags & BAD_BAT_PARAMFLAG_CHECK_HEIGHTREL) || (fabsf(this->actor.playerHeightRel) < 150.0f))) {
        EnBat_SetupDiveAttack(this);
    }
}