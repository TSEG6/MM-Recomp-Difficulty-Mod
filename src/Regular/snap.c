#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_kame.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"

void EnKame_SetupIdle(EnKame* this);
void EnKame_SetupRetreatIntoShell(EnKame* this);
void EnKame_SetupPrepareToAttack(EnKame* this);
void EnKame_SetupEmergeFromShell(EnKame* this);
void EnKame_Struggle(EnKame* this, PlayState* play);

RECOMP_HOOK("EnKame_Update") void SnapUpdate(Actor* thisx, PlayState* play) {

    EnKame* this = (EnKame*)thisx;
    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        if (this->actionFunc == EnKame_Struggle) this->stunTimer--;
        if (this->actionFunc == EnKame_Struggle && this->timer != 0) this->timer--;
        break;

    case 1: {
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;
        if (this->actionFunc == EnKame_Struggle) this->stunTimer--;
        if (this->actionFunc == EnKame_Struggle && this->timer != 0) this->timer = this->timer - 5;
        break;
    }
    default:
        break;
    }
}

RECOMP_HOOK_RETURN("EnKame_SetAttackSpeed") void SnapAtkSpd(EnKame* this) {

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->actor.speed = this->actor.speed * 1.25f;
        break;

    case 1: {
        this->actor.speed = this->actor.speed * 2.0f;
        break;
    }
    default:
        break;
    }
}

RECOMP_PATCH void EnKame_Walk(EnKame* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float AtkDist = 240.0f;

    switch (Difficulty) {
    case 0:
        AtkDist = 360.0f;
        break;

    case 1: {
        AtkDist = 420.0f;
        break;
    }
    default:
        break;
    }

    if ((Player_GetMask(play) != PLAYER_MASK_STONE) && !(player->stateFlags1 & PLAYER_STATE1_800000) &&
        (this->actor.xzDistToPlayer < AtkDist)) {
        EnKame_SetupRetreatIntoShell(this);
        return;
    }

    SkelAnime_Update(&this->snapperSkelAnime);

    if (this->targetRotY != this->actor.shape.rot.y) {
        Math_ScaledStepToS(&this->actor.shape.rot.y, this->targetRotY, 0x100);
        this->actor.world.rot.y = this->actor.shape.rot.y;
    }
    else if (Actor_WorldDistXZToPoint(&this->actor, &this->actor.home.pos) > 40.0f) {
        this->targetRotY = Actor_WorldYawTowardPoint(&this->actor, &this->actor.home.pos) + ((s32)Rand_Next() >> 0x14);
    }

    this->timer--;
    if (this->timer == 0) {
        EnKame_SetupIdle(this);
    }
    else if (Animation_OnFrame(&this->snapperSkelAnime, 0.0f) || Animation_OnFrame(&this->snapperSkelAnime, 15.0f)) {
        Actor_PlaySfx(&this->actor, NA_SE_EN_PAMET_WALK);
    }
}

RECOMP_PATCH void EnKame_RetreatIntoShell(EnKame* this, PlayState* play) {
    Player* player = GET_PLAYER(play);

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float AtkDist = 240.0f;

    switch (Difficulty) {
    case 0:
        AtkDist = 360.0f / 1.5;
        break;

    case 1: {
        AtkDist = 420.0f / 1.5;
        break;
    }
    default:
        break;
    }

    if (SkelAnime_Update(&this->snapperSkelAnime)) {
        if ((Player_GetMask(play) != PLAYER_MASK_STONE) && !(player->stateFlags1 & PLAYER_STATE1_800000) &&
            ((this->timer == 0) || (this->actor.xzDistToPlayer < AtkDist))) {
            EnKame_SetupPrepareToAttack(this);
        }
        else {
            this->timer--;
            if (this->timer == 0) {
                EnKame_SetupEmergeFromShell(this);
            }
        }
    }
    else if (this->snapperSkelAnime.curFrame > 2.0f) {
        this->limbScaleY = 1.5f - ((this->snapperSkelAnime.curFrame - 2.0f) * (7.0f / 30));
        this->limbScaleXZ = 1.5f - ((this->snapperSkelAnime.curFrame - 2.0f) * (1.0f / 12));
    }
    else {
        f32 curFrame = this->snapperSkelAnime.curFrame;

        this->limbScaleY = (0.25f * curFrame) + 1.0f;
        this->limbScaleXZ = (0.25f * curFrame) + 1.0f;
    }
}