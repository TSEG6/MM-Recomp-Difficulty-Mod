#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_bsb.h"

#include "z64rumble.h"
#include "z64shrink_window.h"
#include "attributes.h"

#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "overlays/effects/ovl_Effect_Ss_Hahen/z_eff_ss_hahen.h"
#include "overlays/effects/ovl_Effect_Ss_Hitmark/z_eff_ss_hitmark.h"

#include "assets/objects/gameplay_keep/gameplay_keep.h"

void func_80C0C430(EnBsb* this);
void func_80C0C86C(EnBsb* this);
void func_80C0CA28(EnBsb* this, PlayState* play);
void func_80C0DA58(EnBsb* this);

int TimesArrowed = 0;

typedef enum EnBsbAnimation {
    /*  0 */ ENBSB_ANIM_0,
    /*  1 */ ENBSB_ANIM_1,
    /*  2 */ ENBSB_ANIM_2,
    /*  3 */ ENBSB_ANIM_3,
    /*  4 */ ENBSB_ANIM_4,
    /*  5 */ ENBSB_ANIM_5,
    /*  6 */ ENBSB_ANIM_6,
    /*  7 */ ENBSB_ANIM_7,
    /*  8 */ ENBSB_ANIM_8,
    /*  9 */ ENBSB_ANIM_9,
    /* 10 */ ENBSB_ANIM_10,
    /* 11 */ ENBSB_ANIM_11,
    /* 12 */ ENBSB_ANIM_12,
    /* 13 */ ENBSB_ANIM_13,
    /* 14 */ ENBSB_ANIM_14,
    /* 15 */ ENBSB_ANIM_15,
    /* 16 */ ENBSB_ANIM_16,
    /* 17 */ ENBSB_ANIM_17,
    /* 18 */ ENBSB_ANIM_18,
    /* 19 */ ENBSB_ANIM_19,
    /* 20 */ ENBSB_ANIM_20,
    /* 21 */ ENBSB_ANIM_21,
    /* 22 */ ENBSB_ANIM_22,
    /* 23 */ ENBSB_ANIM_23,
    /* 24 */ ENBSB_ANIM_24,
    /* 25 */ ENBSB_ANIM_MAX
} EnBsbAnimation;

static AnimationHeader* sAnimations[ENBSB_ANIM_MAX] = {
    &object_bsb_Anim_0086BC, // ENBSB_ANIM_0
    &object_bsb_Anim_00CD88, // ENBSB_ANIM_1
    &object_bsb_Anim_000400, // ENBSB_ANIM_2
    &object_bsb_Anim_0065D8, // ENBSB_ANIM_3
    &object_bsb_Anim_000FF0, // ENBSB_ANIM_4
    &object_bsb_Anim_000C50, // ENBSB_ANIM_5
    &object_bsb_Anim_006C48, // ENBSB_ANIM_6
    &object_bsb_Anim_001390, // ENBSB_ANIM_7
    &object_bsb_Anim_002AF4, // ENBSB_ANIM_8
    &object_bsb_Anim_002590, // ENBSB_ANIM_9
    &object_bsb_Anim_007120, // ENBSB_ANIM_10
    &object_bsb_Anim_0043A4, // ENBSB_ANIM_11
    &object_bsb_Anim_007B18, // ENBSB_ANIM_12
    &object_bsb_Anim_001CD8, // ENBSB_ANIM_13
    &object_bsb_Anim_003E1C, // ENBSB_ANIM_14
    &object_bsb_Anim_003238, // ENBSB_ANIM_15
    &object_bsb_Anim_00606C, // ENBSB_ANIM_16
    &object_bsb_Anim_005440, // ENBSB_ANIM_17
    &object_bsb_Anim_004E2C, // ENBSB_ANIM_18
    &object_bsb_Anim_004894, // ENBSB_ANIM_19
    &object_bsb_Anim_004208, // ENBSB_ANIM_20
    &object_bsb_Anim_00D3CC, // ENBSB_ANIM_21
    &object_bsb_Anim_004510, // ENBSB_ANIM_22
    &object_bsb_Anim_001F90, // ENBSB_ANIM_23
    &object_bsb_Anim_00C790, // ENBSB_ANIM_24
};

extern void EnBsb_ChangeAnim(EnBsb*, s32);

extern s32 func_80C0B888(EnBsb*, PlayState*);

extern s32 func_80C0BC30(EnBsb*);

extern void func_80C0B970(EnBsb*, PlayState*);


RECOMP_HOOK("func_80C0C86C") void ResetSpeed(EnBsb* this) {

    this->skelAnime.playSpeed = 1.0f;
}

RECOMP_PATCH void func_80C0C6A8(EnBsb* this, PlayState* play) {
    f32 curFrame = this->skelAnime.curFrame;

    int Difficulty = (int)recomp_get_config_double("diff_option");
    int ArrowAnimPlay = 2;
    int ArrowAnimPlayTimes = 5;

    switch (Difficulty) {
    case 0:
        ArrowAnimPlay = 1;
        ArrowAnimPlayTimes = 4;
        this->skelAnime.playSpeed = 1.2f;
        break;

    case 1:
        ArrowAnimPlay = 0;
        ArrowAnimPlayTimes = 2;
        this->skelAnime.playSpeed = 1.3f;
        break;

    default:
        break;
    }

    if (TimesArrowed > ArrowAnimPlayTimes) func_80C0C430(this);
    else if (play->gameplayFrames % 20 == 0) TimesArrowed++;

    func_80C0B888(this, play);

    if ((!this->unk_02DC || (this->unk_02DC && (this->animIndex == ENBSB_ANIM_2) && (curFrame >= this->animEndFrame) &&
        (this->unk_0294 == 0))) &&
        ((this->drawDmgEffType == ACTOR_DRAW_DMGEFF_FROZEN_SFX) ||
            (this->drawDmgEffType == ACTOR_DRAW_DMGEFF_FROZEN_NO_SFX)) &&
        (this->drawDmgEffTimer != 0)) {
        Actor_SpawnIceEffects(play, &this->actor, this->bodyPartsPos, ENBSB_BODYPART_MAX, 2,
            this->drawDmgEffFrozenSteamScale, 0.4f);
        this->drawDmgEffTimer = 0;
        this->drawDmgEffType = ACTOR_DRAW_DMGEFF_FIRE;
    }

    if ((this->animIndex == ENBSB_ANIM_2) && (curFrame >= this->animEndFrame) && (this->unk_0294 == 0)) {
        if (!this->unk_02DC) {
            EnBsb_ChangeAnim(this, ENBSB_ANIM_3);
        }
        else {
            func_80C0C86C(this);
        }
    }
    else if (this->animIndex == ENBSB_ANIM_3) {
        Math_SmoothStepToS(&this->actor.world.rot.y, this->actor.yawTowardsPlayer, 1, 0x7D0, 0);
        if (Animation_OnFrame(&this->skelAnime, 7.0f) || Animation_OnFrame(&this->skelAnime, 15.0f)) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_KTIA_PAUSE_K);
        }

        if (curFrame >= this->animEndFrame) {
            this->unk_02A4++;
            if (this->unk_02A4 >= ArrowAnimPlay) {
                func_80C0C430(this);
                this->skelAnime.playSpeed = 1.0f;
            }
        }
    }
}

RECOMP_HOOK("func_80C0D10C") void AttackBonus(EnBsb* this, PlayState* play) {
    f32 curFrame = this->skelAnime.curFrame;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    switch (Difficulty) {
    case 0:
        this->skelAnime.playSpeed = 2.5f;
        break;

    case 1:
        this->skelAnime.playSpeed = 5.0f;

        if (curFrame >= this->animEndFrame) {
            if ((this->actor.world.pos.z > -1300.0f) || (this->actor.colChkInfo.health < 20)) {
                func_80C0CA28(this, play);
            }
            else {
                func_80C0C86C(this);
            }
        }
        break;

    default:
        break;
    }

}

RECOMP_PATCH void func_80C0C484(EnBsb* this, PlayState* play) {
    f32 sp34 = 9999.0f;
    s16 var_a1;
    s16 temp_v1;

    func_80C0BC30(this);

    int Difficulty = (int)recomp_get_config_double("diff_option");
    float NewSpeed = 3.5f;
    s16 additionalROT = 0;

    switch (Difficulty) {
    case 0:
        NewSpeed = 3.75f;
        additionalROT = 0x50;
        break;

    case 1:
        NewSpeed = 4.25f;
        additionalROT = 0x100;
        break;

    default:
        break;
    }

    if (func_80C0B888(this, play)) {
        this->actor.flags &= ~ACTOR_FLAG_UPDATE_DURING_OCARINA;
        func_80C0C86C(this);
        return;
    }

    var_a1 = this->actor.yawTowardsPlayer;

    if (this->unk_0294 == 1) {
        this->actor.flags &= ~ACTOR_FLAG_UPDATE_DURING_OCARINA;
    }

    if (this->path != NULL) {
        var_a1 = SubS_GetDistSqAndOrientPath(this->path, this->waypoint, &this->actor.world.pos, &sp34);
    }

    Math_SmoothStepToS(&this->actor.world.rot.y, var_a1, 2, 0x2EE + additionalROT, 5);

    temp_v1 = ABS_ALT((s16)(this->actor.world.rot.y - var_a1));

    if (temp_v1 < 0x1000) {
        this->unk_02BC = this->actor.world.rot.y;
        this->actor.speed = NewSpeed;
        if (!this->unk_02AF) {
            this->unk_02AF = true;
            Audio_PlayBgm_StorePrevBgm(NA_BGM_CHASE);
            TimesArrowed = 0;
        }
    }

    func_80C0B970(this, play);

    if (sp34 < SQ(5.0f)) {
        if (this->path != NULL) {
            this->waypoint++;
            if (this->waypoint >= this->path->count) {
                this->waypoint--;
                func_80C0DA58(this);
            }
        }
    }
}

RECOMP_HOOK("EnBsb_Destroy") void KeetaGONEPlusSkillIssue(Actor* thisx, PlayState* play) {

    TimesArrowed = 0;
}

RECOMP_HOOK("EnBsb_Update") void KeetaUpdate(Actor* thisx, PlayState* play) {

	EnBsb* this = (EnBsb*)thisx;

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