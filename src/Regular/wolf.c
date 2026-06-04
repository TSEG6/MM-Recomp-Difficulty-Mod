#include "modding.h"
#include "global.h"
#include "recomputils.h"
#include "recompconfig.h"
#include "z_en_wf.h"
#include "overlays/actors/ovl_En_Bom_Chu/z_en_bom_chu.h"
#include "overlays/actors/ovl_En_Clear_Tag/z_en_clear_tag.h"
#include "overlays/actors/ovl_Obj_Ice_Poly/z_obj_ice_poly.h"

void func_80991200(EnWf* this);
void func_80991948(EnWf* this);
void func_8099223C(EnWf* this);
void func_809922B4(EnWf* this, PlayState* play);
void func_80992A74(EnWf* this, PlayState* play);
void func_80992068(EnWf* this, PlayState* play);

extern s32 func_80990948(PlayState*, EnWf*, s16);
extern void func_80990C6C(EnWf*, PlayState*, s32);

// Speed and defense
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

// Lower timer for attacking (seems useless)
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

// Attacking lunge & removal of turn around on Hard
RECOMP_PATCH void func_80991C80(EnWf* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s32 sp30;
    s32 onAnim15thFrame;
    s16 sp2A;

    int Difficulty = (int)recomp_get_config_double("diff_option");

    sp2A = BINANG_SUB(player->actor.shape.rot.y, this->actor.shape.rot.y);
    sp30 = ABS_ALT(BINANG_SUB(this->actor.yawTowardsPlayer, this->actor.shape.rot.y));

    if (((this->skelAnime.curFrame >= 9.0f) && (this->skelAnime.curFrame < 13.0f)) ||
        ((this->skelAnime.curFrame >= 17.0f) && (this->skelAnime.curFrame < 20.0f))) {

        if (Difficulty == 1) this->actor.speed = 6.0f;
        if (Difficulty == 0) this->actor.speed = 4.0f;

        if (Difficulty >= 1) {
            Math_ScaledStepToS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 0x800);
            this->actor.world.rot.y = this->actor.shape.rot.y;
        }

        if (!(this->collider1.base.atFlags & AT_ON)) {
            Actor_PlaySfx(&this->actor, NA_SE_EN_WOLFOS_ATTACK);
        }
        this->collider1.base.atFlags |= AT_ON;
    }
    else {
        this->actor.speed = 0.0f;
        this->collider1.base.atFlags &= ~AT_ON;
    }

    onAnim15thFrame = Animation_OnFrame(&this->skelAnime, 15.0f);

    if ((onAnim15thFrame && !Actor_IsTargeted(play, &this->actor) &&
        (!Actor_IsFacingPlayer(&this->actor, 0x2000) || (this->actor.xzDistToPlayer >= 100.0f))) ||
        SkelAnime_Update(&this->skelAnime)) {

        if (!onAnim15thFrame && (this->unk_2A0 != 0)) {
            this->actor.shape.rot.y += TRUNCF_BINANG(0xCCC * (1.5f + ((this->unk_2A0 - 4) * 0.4f)));
            func_80990C6C(this, play, 1);
            this->unk_2A0--;
        }
        else if (!Actor_IsFacingPlayer(&this->actor, 0x1554) && !onAnim15thFrame) {

            if (Difficulty >= 1) {
                func_80992A74(this, play);
            }
            else {
                func_80991200(this);
                this->unk_2A0 = (s32)Rand_ZeroFloat(5.0f) + 5;
                if (sp30 >= 0x32C9) {
                    this->unk_298 = 7;
                }
            }
        }
        else if ((Rand_ZeroOne() > 0.7f) || (this->actor.xzDistToPlayer >= 120.0f)) {

            if (Difficulty >= 1) {
                func_80992A74(this, play);
            }
            else {
                func_80991200(this);
                this->unk_2A0 = (s32)Rand_ZeroFloat(5.0f) + 5;
            }
        }
        else {
            this->actor.world.rot.y = this->actor.yawTowardsPlayer;
            if (Rand_ZeroOne() > 0.7f) {
                func_80992A74(this, play);
            }
            else if (ABS_ALT(sp2A) < 0x2711) {
                if (sp30 >= 0x3E81) {
                    this->actor.world.rot.y = this->actor.yawTowardsPlayer;
                    func_80991948(this);
                }
                else {
                    func_80990948(play, this, true);
                }
            }
            else {
                func_80991948(this);
            }
        }
    }
}