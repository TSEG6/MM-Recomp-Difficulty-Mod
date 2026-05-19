#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_rr.h"
#include "z64rumble.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "assets/objects/object_rr/object_rr.h"

void func_808FB088(EnRr* this, PlayState* play);

void func_808FA238(EnRr* this, f32 arg1) {
    this->actor.speed = arg1;
    Actor_PlaySfx(&this->actor, NA_SE_EN_LIKE_WALK);
}

void func_808FA260(EnRr* this) {
    static f32 D_808FC1E4[] = { 0.0f, 500.0f, 750.0f, 1000.0f, 1000.0f };
    s32 i;

    this->unk_1E1 = 1;
    this->unk_1E6 = 20;
    this->unk_1F6 = 2500;
    this->unk_210 = 0.0f;

    for (i = 0; i < ARRAY_COUNT(this->unk_324); i++) {
        this->unk_324[i].unk_04 = D_808FC1E4[i];
        this->unk_324[i].unk_14 = 6000;
        this->unk_324[i].unk_18 = 0;
        this->unk_324[i].unk_0C = 0.8f;
    }

    this->actionFunc = func_808FB088;

    Actor_PlaySfx(&this->actor, NA_SE_EN_LIKE_UNARI);
}

RECOMP_HOOK("EnRr_Update") void LLUpdate(Actor* thisx, PlayState* play) {

	EnRr* this = (EnRr*)thisx;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float speedMultiplier = 1.0f;

    switch (Difficulty) {
    case 0:
        speedMultiplier = 1.75f;
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage) / 2;
        break;

    case 1:
        speedMultiplier = 2.5f;
        this->actor.colChkInfo.damage = (this->actor.colChkInfo.damage + 2) / 3;
        break;

    default:
        break;
    }

    if (this->actor.colChkInfo.health != 0) {

        if (this->actor.speed <= 0.66f) {
            this->actor.speed *= speedMultiplier;
        }
    }

    if (this->actor.colChkInfo.health == 0) this->actor.speed = 0;
}

RECOMP_HOOK("func_808FAF94") void Chase(EnRr* this, PlayState* play) {

    Player* player = GET_PLAYER(play);

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float Multiplier = 1.0f;

    switch (Difficulty) {
    case 0:
        Multiplier = 1.5f;
        break;

    case 1:
        Multiplier = 2.0f;
        break;

    default:
        break;
    }

    Math_SmoothStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 10, 0x1F4, 0);
    this->actor.world.rot.y = this->actor.shape.rot.y;
    if ((this->unk_1E6 == 0) && !(player->stateFlags2 & PLAYER_STATE2_80) &&
        (Player_GetMask(play) != PLAYER_MASK_STONE) &&
        (this->actor.xzDistToPlayer < (8421.053f * this->actor.scale.x) * Multiplier)) {
        func_808FA260(this);
    }
    else if ((this->actor.xzDistToPlayer < 400.0f) && (this->actor.speed == 0.0f)) {
        func_808FA238(this, 2.0f);
    }
}